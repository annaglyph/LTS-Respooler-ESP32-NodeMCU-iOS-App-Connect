#include <NimBLEDevice.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <math.h>

// ------------------- BLE UUIDs -----------------------------------
#define SERVICE_UUID        "9E05D06D-68A7-4E1F-A503-AE26713AC101"
#define CHARACTERISTIC_UUID "7CB2F1B4-7E3F-43D2-8C92-DF58C9A7B1A8"

// ------------------------ Board Info ----------------------------
#define FIRMWARE_VERSION "0.0.1"
#define BOARD_NAME "esp32 PCB"

// --------------------- Hardware Pin Defines ---------------------
#define LED1_PIN            16
#define LED2_PIN            17
#define LED_CONN_PIN        2
#define FILAMENT_PIN        18
#define STEP_PIN            23
#define DIR_PIN             5
#define EN_PIN              19
#define BUTTON_PIN          21

#define START_INTERVAL      700
#define DEFAULT_INTERVAL    146

// ------------------- Direct settings variables -------------------
int speedPercent = 80;                         // 50-100
bool deviceConnected = false;
bool isMotorRunning = false;                    // true, false
bool shouldStartMotorNow = false;
bool filamentDetected = false;
bool useFilamentSensor = true;
int motorDirection = 0;
int ledBrightness = 50;                         // 0-100
int pwmValue = 0;
int jingleStyle = 1;
int triggerJingleNow = 0;
char currentState = 'I';

bool pendingDirectionChange = false;
int newMotorDirection = 0;
unsigned long delayStartUntil = 0;
unsigned long filamentLostSince = 0;

NimBLECharacteristic* pCharacteristic = nullptr;
Preferences prefs;

unsigned long lastNotify = 0;
unsigned long lastStep = 0;

unsigned long lastLedToggle = 0;
unsigned long lastAccelUpdate = 0;
unsigned long lastConnLedTime = 0;
bool stepState = false;
bool ledState = false;

unsigned long stepIntervalMicros = START_INTERVAL;
unsigned long calibrationAt80Speed = 375000; // z. B. 6:15 Minuten bei 80 %
unsigned long spoolingStartTime = 0;
float progress = 0.0;
unsigned long totalEstimatedTime = 0;
unsigned long targetStepIntervalMicros = DEFAULT_INTERVAL;
const unsigned long accelUpdateInterval = 20;
const int accelStep = 5;

bool lastStableState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

int calculatePWM(int brightnessPercent) {
    float gamma = 2.2;
    float normalized = constrain(brightnessPercent, 0, 100) / 100.0;
    return round(pow(normalized, gamma) * 255);
}

// ================== sendStatus ==================

void sendStatus(bool forceSend = false) {
  int chipTemp = (int)temperatureRead();

  StaticJsonDocument<256> doc;
  doc["STAT"]    = String(currentState);
  doc["HAS_FIL"] = filamentDetected;
  doc["USE_FIL"] = useFilamentSensor;
  doc["PROG"]    = progress;
  doc["REM"]     = totalEstimatedTime / 1000;  // seconds
  doc["SPD"]     = speedPercent;
  doc["JIN"]     = jingleStyle;
  doc["LED"]     = ledBrightness;
  doc["DIR"]     = motorDirection;
  doc["FW"]      = FIRMWARE_VERSION;
  doc["TEMP"]    = chipTemp;

  String out;
  serializeJson(doc, out);

  if (deviceConnected && pCharacteristic) {
    pCharacteristic->setValue(out.c_str());
    pCharacteristic->notify();
  }
}

void sendDone() {
    if (!deviceConnected || !pCharacteristic) return;
    pCharacteristic->setValue("D");
    pCharacteristic->notify();
}

void playStepperJingle() {
    if (jingleStyle == 0) return;

    delay(500);
    digitalWrite(EN_PIN, LOW);

    if (jingleStyle == 1) { // einfach
        int freqs[] = {1000, 1200, 800};
        int toneLengths[] = {35, 40, 200};
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < toneLengths[i]; j++) {
                digitalWrite(STEP_PIN, HIGH);
                delayMicroseconds(freqs[i]);
                digitalWrite(STEP_PIN, LOW);
                delayMicroseconds(freqs[i]);
            }
            delay(30);
        }
    } else if (jingleStyle == 2) { // glissando
        for (int f = 1700; f >= 700; f -= 70) {
            for (int j = 0; j < 20; j++) {
                digitalWrite(STEP_PIN, HIGH);
                delayMicroseconds(f);
                digitalWrite(STEP_PIN, LOW);
                delayMicroseconds(f);
            }
        }
    } else if (jingleStyle == 3) { // Star Wars
    int freqs[] = {440, 440, 440, 349, 523, 440, 349, 523, 440};
    int lengths[] = {80, 80, 80, 50, 30, 80, 50, 30, 100};

    for (int i = 0; i < 9; i++) {
        int periodMicros = 1000000 / freqs[i];
        for (int j = 0; j < lengths[i]; j++) {
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(periodMicros);
            digitalWrite(STEP_PIN, LOW);
            delayMicroseconds(periodMicros);
        }
        delay(40);
    }
}
    digitalWrite(EN_PIN, HIGH);
}

// ================== handleCommand ==================

void handleCommand(const std::string& cmd) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, cmd.c_str());
  if (error) {
    Serial.println(F("JSON parse failed"));
    return;
  }

  if (doc.containsKey("CMD")) {
    String command = doc["CMD"].as<String>();

    if (command == "START") {
      if (!isMotorRunning) {
        shouldStartMotorNow = true;
        delayStartUntil = millis();
      }
    } 
    else if (command == "STOP") {
      if (isMotorRunning) {
        isMotorRunning = false;
        digitalWrite(EN_PIN, HIGH);   // disable driver
        digitalWrite(STEP_PIN, LOW);
        currentState = 'I';  // stop stepping
        sendStatus(true);                   // notify client
      }
    }
    else if (command == "PAUSE") {
        if (isMotorRunning) {
            isMotorRunning = false;
            digitalWrite(EN_PIN, HIGH);
            digitalWrite(STEP_PIN, LOW);
            unsigned long elapsed = 0;
            if (spoolingStartTime > 0) {
                elapsed = millis() - spoolingStartTime;
            }
            //pausedElapsed = elapsed;
            spoolingStartTime = 0;
            currentState = 'P';
            sendStatus(true);
        }
    }
  }
  if (doc.containsKey("SET")) {
    JsonObject set = doc["SET"];
  
    if (set.containsKey("USE_FIL")) {
      useFilamentSensor = set["USE_FIL"];
      prefs.begin("respooler", false);           // save to flash
      prefs.putBool("use_fil", useFilamentSensor);
      prefs.end();
    }
    if (set.containsKey("DIR")) {
      int dir = set["DIR"];
      if (dir == 0 || dir == 1) {  // make sure it’s valid
        motorDirection = dir;
        digitalWrite(DIR_PIN, motorDirection);

        // Save to flash
        prefs.begin("respooler", false);
        prefs.putUInt("dir", motorDirection);
        prefs.end();
      }
    }
    if (set.containsKey("SPD")) {
      int speed = constrain((int)set["SPD"], 50, 100);
      targetStepIntervalMicros = map(speed, 50, 100, 170, 93);
      speedPercent = speed;
      prefs.begin("respooler", false);
      prefs.putUInt("speed", targetStepIntervalMicros);
      prefs.end();
    }
    if (set.containsKey("LED")) {
      ledBrightness = constrain((int)set["LED"], 0, 100);
      pwmValue = calculatePWM(ledBrightness);
      prefs.begin("respooler", false);
      prefs.putUInt("led", ledBrightness);
      prefs.end();
    }
  }
}

class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo&) override {
        deviceConnected = true;
    }
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo&, int) override {
        deviceConnected = false;
        NimBLEDevice::startAdvertising();
    }
};

class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo&) override {
        std::string rx = pChar->getValue();
        if (!rx.empty()) handleCommand(rx);
    }
};

void setup() {
    pinMode(FILAMENT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(EN_PIN, OUTPUT); digitalWrite(EN_PIN, HIGH);
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(LED_CONN_PIN, OUTPUT);

    prefs.begin("respooler", true);
    targetStepIntervalMicros = prefs.getUInt("speed", DEFAULT_INTERVAL);
    motorDirection = prefs.getUInt("dir", 0);
    useFilamentSensor = prefs.getBool("use_fil", false);
    ledBrightness = prefs.getUInt("led", 50);
    jingleStyle = prefs.getUInt("jingle", 1);
    calibrationAt80Speed = prefs.getULong("cal80", 375000);
    prefs.end();
    // Prüfe den Kalibrierwert und setze ggf. Standardwert
    if (calibrationAt80Speed < 10000) calibrationAt80Speed = 375000;
    // Debug-Ausgaben (nur sinnvoll bei aktivem Serial Monitor)
    Serial.begin(115200);
    Serial.println("Kalibrierung 80%: " + String(calibrationAt80Speed));
    pwmValue = calculatePWM(ledBrightness);

    NimBLEDevice::init(BOARD_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P6);
    NimBLEDevice::setMTU(512);
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    pService->start();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    NimBLEAdvertisementData advData;
    //pAdvertising->addServiceUUID(SERVICE_UUID);
    advData.setName(BOARD_NAME);
    advData.addServiceUUID(SERVICE_UUID);
    pAdvertising->setAdvertisementData(advData);
    pAdvertising->start();

    int startupPWM = calculatePWM(50);
    analogWrite(LED1_PIN, startupPWM);
    analogWrite(LED2_PIN, startupPWM);
    delay(500);
    analogWrite(LED1_PIN, 0);
    analogWrite(LED2_PIN, 0);
    analogWrite(LED_CONN_PIN, 0);
    currentState = 'I';
}

void loop() {
    unsigned long now = millis();

    bool currentFilament = digitalRead(FILAMENT_PIN) == LOW;
    if (currentFilament) filamentLostSince = 0;
    else if (filamentDetected && filamentLostSince == 0) filamentLostSince = now;
    filamentDetected = currentFilament;

    if (isMotorRunning) {
        if (now - lastLedToggle > 1000) {
            lastLedToggle = now;
            ledState = !ledState;
        }
        analogWrite(LED1_PIN, ledState ? pwmValue : 0);
    } else {
        analogWrite(LED1_PIN, 0);
    }

    if (useFilamentSensor)
        analogWrite(LED2_PIN, filamentDetected ? pwmValue : 0);
    else
        analogWrite(LED2_PIN, isMotorRunning ? (ledState ? pwmValue : 0) : 0);

    if (isMotorRunning && useFilamentSensor && !filamentDetected && filamentLostSince > 0 && now - filamentLostSince > 100) {
        isMotorRunning = false;
        digitalWrite(EN_PIN, HIGH);
        digitalWrite(STEP_PIN, LOW);
        currentState = 'D';
        sendStatus(true);                // zuerst Done senden
        playStepperJingle();      // dann Jingle spielen
        progress = 0.0;
        totalEstimatedTime = 0;
    }

    if (pendingDirectionChange && !isMotorRunning) {
        motorDirection = newMotorDirection;
        prefs.begin("respooler", false);
        prefs.putUInt("dir", motorDirection);
        prefs.end();
        delayStartUntil = now + 1000;
        shouldStartMotorNow = true;
        pendingDirectionChange = false;
    }

    if (shouldStartMotorNow && now >= delayStartUntil) {
        if (!useFilamentSensor || filamentDetected) {
            digitalWrite(DIR_PIN, motorDirection);
            digitalWrite(EN_PIN, LOW);
            stepIntervalMicros = START_INTERVAL;
            lastAccelUpdate = now;
            isMotorRunning = true;
            currentState = 'R';
            sendStatus(true);
            spoolingStartTime = now;
            // Berechne geschätzte Zeit auf Basis des Kalibrierwerts bei 80%
            speedPercent = map(targetStepIntervalMicros, 200, 100, 50, 100);
            totalEstimatedTime = calibrationAt80Speed * (80.0 / speedPercent);
        }
        shouldStartMotorNow = false;
    }

    bool reading = digitalRead(BUTTON_PIN);
    if (reading != lastStableState) {
        if (now - lastDebounceTime > debounceDelay) {
            if (reading == LOW) {
                if (!isMotorRunning) {
                    if (useFilamentSensor && !filamentDetected) {
                        for (int i = 0; i < 5; i++) {
                            analogWrite(LED1_PIN, pwmValue);
                            analogWrite(LED2_PIN, pwmValue);
                            delay(100);
                            analogWrite(LED1_PIN, 0);
                            analogWrite(LED2_PIN, 0);
                            delay(100);
                        }
                    } else {
                        shouldStartMotorNow = true;
                        delayStartUntil = now;
                    }
                } else {
                    isMotorRunning = false;
                    digitalWrite(EN_PIN, HIGH);
                    digitalWrite(STEP_PIN, LOW);
                    currentState = 'P';
                    sendStatus(true);
                }
            }
            lastStableState = reading;
        }
    } else {
        lastDebounceTime = now;
    }

    if (deviceConnected && now - lastNotify > 200) {
        sendStatus();
        lastNotify = now;
    }

    if (isMotorRunning && now - lastAccelUpdate > accelUpdateInterval) {
        lastAccelUpdate = now;
        if (stepIntervalMicros > targetStepIntervalMicros) {
            stepIntervalMicros -= accelStep;
            if (stepIntervalMicros < targetStepIntervalMicros)
                stepIntervalMicros = targetStepIntervalMicros;
        } else if (stepIntervalMicros < targetStepIntervalMicros) {
            stepIntervalMicros += accelStep;
            if (stepIntervalMicros > targetStepIntervalMicros)
                stepIntervalMicros = targetStepIntervalMicros;
        }
        // Aktualisiere totalEstimatedTime basierend auf aktueller Geschwindigkeit
        speedPercent = map(stepIntervalMicros, 200, 100, 50, 100);
        unsigned long newTotal = calibrationAt80Speed * (80.0 / speedPercent);
        // Passe spoolingStartTime entsprechend an, um den Fortschritt zu erhalten
        unsigned long elapsed = millis() - spoolingStartTime;
        float progressRatio = (totalEstimatedTime > 0) ? (float)elapsed / totalEstimatedTime : 0.0;
        spoolingStartTime = millis() - (newTotal * progressRatio);
        totalEstimatedTime = newTotal;
    }

    if (isMotorRunning) {
        unsigned long nowMicros = micros();
        if (nowMicros - lastStep >= stepIntervalMicros) {
            lastStep = nowMicros;
            stepState = !stepState;
            digitalWrite(STEP_PIN, stepState);
        }
    } else {
        digitalWrite(STEP_PIN, LOW);
    }

    if (deviceConnected) {
        analogWrite(LED_CONN_PIN, 70);
    } else {
        if (now - lastConnLedTime > 50) {
            lastConnLedTime = now;
            static int pulse = 0;
            static int dir = 1;
            pulse += dir * 2;
            if (pulse >= 60 || pulse <= 0) dir *= -1;
            analogWrite(LED_CONN_PIN, pulse);
        }
    }

    if (triggerJingleNow != 0) {
        playStepperJingle();
        triggerJingleNow = 0;
    }
}