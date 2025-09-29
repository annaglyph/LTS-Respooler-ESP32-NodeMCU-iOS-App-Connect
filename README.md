# LTS Respooler – ESP32 NodeMCU (WEMOS D1 Mini) Edition

This project is a **stripped-down version** of the [LTS Respooler](https://github.com/LukasT03/LTS-Respooler), adapted to run on the **WEMOS D1 Mini ESP32 (ESP-WROOM-32)**.  

The LTS Respooler is a motorized tool for **respolling 3D printer filament**. It connects via **Bluetooth** to the official **LTS iOS app** or the [LTS Design Web App](https://lts-design.com/pages/software), allowing you to start/stop spooling, change direction, and monitor progress.

This fork keeps the **core functionality** but removes advanced features to fit and run reliably on a smaller ESP32 NodeMCU board. The majority of the code was originally written by [Lukas Tuchscherer](https://github.com/LukasT03) while the **LTS iOS app** was in TestFlight.

---

## 📑 Table of Contents
- [Changes from the Original Project](#-changes-from-the-original-project)
- [Hardware Setup](#-hardware-setup)
- [Getting Started](#-getting-started)
- [Using the Respooler](#-using-the-respooler)
- [Credits](#-credits)

---

## ✂️ Changes from the Original Project

The full-feature [LTS Respooler firmware](https://github.com/LukasT03/LTS-Respooler) includes OTA updates, WiFi scanning, fan control, LED effects, torque monitoring, and adjustable motor strength.  

This simplified fork removes:
- OTA updates  
- WiFi scanning and connection  
- Fan control  
- LED status patterns (basic LED feedback only)  
- Torque monitoring  
- Motor strength adjustments  

Everything else, including **Bluetooth connectivity to the LTS apps**, remains supported.

---

## 🔧 Hardware Setup

### Core Components
- **Board:** WEMOS D1 Mini ESP32 (ESP-WROOM-32 module)  
- **Motor driver:** TMC2209 (via UART)  
- **Filament sensor** (optional, for detecting filament presence)  
- **Mini PCB breadboard** (2.0" × 1.5") for soldering headers and connections  

### Supporting Electronics
- **1×** L7805 Linear Positive Voltage Regulator (5V)  
- **1×** 100 µF 25 V Electrolytic Capacitor  
- **1×** 10 µF 25 V Electrolytic Capacitor  
- **2×** 220 Ω ¼ W Carbon Film Resistors  
- **1×** Power Jack Plug – Female 5.5 mm × 2.1 mm (12 V input)  

![Wiring Diagram](docs/circuit-diagram.png)

![Soldered PCB Breadboard](docs/soldered-pcb-breadboard.jpg)

---

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
Install the following via Arduino Library Manager:
- **TMCStepper**  
- **NimBLE-Arduino**  
- **ArduinoJson**  
- **Adafruit NeoPixel**  

(Core ESP32 libraries like WiFi and Preferences are included with the ESP32 Arduino core.)

### Flashing the Firmware
1. Clone or download this repository.  
2. Open the `.ino` file in Arduino IDE.  
3. Select your board and upload settings.  
4. Click **Upload** to flash the firmware.  

---

## 📱 Using the Respooler

1. Power on your ESP32 NodeMCU build.  
2. Open the **LTS iOS app** or the [LTS Design Web App](https://lts-design.com/pages/software).  
3. Pair via **Bluetooth**.  
4. Use the app to:
   - Start or stop spooling
   - Adjust speed of motor  
   - Change motor direction
   - Adjust LED brightness
   - Enable filament sensor use  
   - Monitor progress, filament sensor status, and Bluetooth connection
   - Monitor ESP32 chip temperature  

---

## 🙏 Credits

- Original code and full-feature firmware: [**LukasT03/LTS-Respooler**](https://github.com/LukasT03/LTS-Respooler)  
- 3D print files: [**LTS Respooler Motorized Filament Winder**](https://makerworld.com/en/models/448008-lts-respooler-motorized-filament-winder#profileId-1962243)  
- Hardware kits and info: [**LTS Design**](https://lts-design.com/)  
- ESP32 NodeMCU adaptation: [annaglyph](https://github.com/annaglyph)  

⚠️ This fork is provided without an explicit license. The **original LTS Respooler code does not specify a license**, meaning it is “all rights reserved” by default. Use responsibly and with attribution.

