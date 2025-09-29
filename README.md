# LTS Respooler – ESP32 NodeMCU (WEMOS D1 Mini) Edition

This project is a **stripped-down version** of the [LTS Respooler](https://github.com/LukasT03/LTS-Respooler), adapted to run on the **WEMOS D1 Mini ESP32 (ESP-WROOM-32)**.  

The LTS Respooler is a motorized tool for **respolling 3D printer filament**. It connects via **Bluetooth** to the official **LTS iOS app** or the [LTS Design Web App](https://lts-design.com/pages/software), allowing you to start/stop spooling, change direction, and monitor progress.

This version keeps the **core functionality** but removes some of the advanced features so it will fit and run reliably on a smaller ESP32 NodeMCU board.

The majority of the code was originally written by [Lukas Tuchscherer](https://github.com/LukasT03) while the **LTS iOS app** was in TestFlight. 

---

## ✂️ Changes from the Original Project

The current [LTS Respooler](https://github.com/LukasT03/LTS-Respooler) firmware includes OTA updates, WiFi scanning, fan control, LED effects, torque monitoring, and adjustable motor strength.  

In order to fit the sketch on the WEMOS D1 Mini ESP32, this fork **does not implement**:
- OTA updates  
- WiFi scanning and connection  
- Fan control  
- LED status patterns (basic LED feedback only)  
- Torque monitoring  
- Motor strength adjustments  

Everything else, including **Bluetooth connectivity to the LTS apps**, remains supported.

---

## 🔧 Hardware Setup

- **Board:** WEMOS D1 Mini ESP32 (ESP-WROOM-32 module)  
- **Motor driver:** TMC2209 (via UART)  
- **Filament sensor** (optional, for detecting filament presence)  
- **Mini PCB breadboard** for soldering headers and connections 2.0"x1.5" 
- **1x** - L7805 Linear Positive Voltage Regulator 5V
- **1x** - 100uF 25v Electrolytic Capacitor
- **1x** - 10uF 25v Electrolytic Capacitor
- **2x** - 220Ω (Ohm) 1/4W Carbon Film Resistor
- **1x** - Power Jack Plug - Female 5.5mm x 2.1mm 12V

![Wiring Diagram](docs/circuit-diagram.png)

---

## 🙏 Credits

- Original code project and full-feature firmware: [**LukasT03/LTS-Respooler**](https://github.com/LukasT03/LTS-Respooler)
- 3D print files: [**LTS Respooler Motorized Filament Winder**](https://makerworld.com/en/models/448008-lts-respooler-motorized-filament-winder#profileId-1962243)
- Hardware kits and additional information: [**LTS Design**](https://lts-design.com/)
- ESP32 NodeMCU adaptation: [annaglyph](https://github.com/annaglyph)  

This fork is provided without an explicit license. Please note that the **original LTS Respooler code does not specify a license**, which means it is “all rights reserved” by default. Use responsibly and with attribution.

---