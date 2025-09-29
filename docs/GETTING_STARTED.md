## 🚀 Getting Started

### Requirements
- [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x or 2.x)  
- [ESP32 Arduino core](https://github.com/espressif/arduino-esp32) installed  
- USB cable for flashing the WEMOS D1 Mini ESP32  

### Arduino IDE Settings
- **Board:** `ESP32 Dev Module`  
- **Upload Speed:** `230400`  
- **Partition Scheme:** `Default`  

### Installing Libraries
Make sure you install the following libraries via the Arduino Library Manager:
- **TMCStepper**  
- **NimBLE-Arduino**  
- **ArduinoJson**  
- **Adafruit NeoPixel**  

(Other core libraries like WiFi and Preferences come with the ESP32 Arduino core.)

### Flashing the Firmware
1. Clone or download this repository.  
2. Open the `.ino` file in Arduino IDE.  
3. Select your board and upload settings.  
4. Click **Upload** to flash the firmware.  

---

## 📱 Using the Respooler

- Power on your ESP32 NodeMCU build.  
- Open the **LTS iOS app** or the [web control app](https://lts-design.com/pages/software).  
- Pair via **Bluetooth**.  
- Use the app to:
  - Start or stop spooling  
  - Change motor direction  
  - Monitor progress and filament sensor status  

---

## 🙏 Credits

- Original project and full-feature firmware: [**LukasT03/LTS-Respooler**](https://github.com/LukasT03/LTS-Respooler)  
- ESP32 NodeMCU adaptation: [annaglyph](https://github.com/annaglyph)  

This fork is provided without an explicit license. Please note that the **original LTS Respooler code does not specify a license**, which means it is “all rights reserved” by default. Use responsibly and with attribution.

---
