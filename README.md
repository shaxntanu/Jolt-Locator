# Jolt Locator – ESP32 Energy Drink Store Locator ⚡

> **My first PCB design using KiCad** - A fun GPS-based device to help you find your nearest Jolt energy drink store!

![KiCad](https://img.shields.io/badge/KiCad-9.0.6-blue?logo=kicad)
![Arduino](https://img.shields.io/badge/Arduino-ESP32-00979D?logo=arduino)
![ESP32](https://img.shields.io/badge/MCU-ESP32-red)
![License](https://img.shields.io/badge/License-MIT-green)

---

## 🎯 About This Project

**Need a Jolt? Let this device guide you there!**

The Jolt Locator is a handheld GPS device designed to help energy drink enthusiasts find nearby stores that sell Jolt energy drinks. Using GPS to track your current location and a digital compass to point you in the right direction, you'll never be far from your next caffeine fix.

This project also marks my **first venture into PCB design** using KiCad. I wanted to combine learning hardware design with something fun and practical - and what's more practical than finding energy drinks when you need them?

**Why this project?**
- Wanted a fun, themed project to learn PCB design
- GPS + compass navigation combines multiple communication protocols (I2C, UART, GPIO)
- Perfect complexity level for a first PCB - challenging but achievable
- Creates something unique and conversation-worthy

**The Journey:**
Starting with zero KiCad experience, I learned schematic symbols, footprint assignment, PCB routing, design rules, and the entire workflow from concept to manufacturable design. The energy drink theme kept me motivated throughout the learning process!

---

## 📋 Overview

The **Jolt Locator** is a handheld energy drink store finder built around the ESP32 microcontroller. It provides:

- **Real-time GPS tracking** - Know exactly where you are
- **Digital compass** - Get oriented and walk in the right direction toward stores
- **OLED display** - Shows your coordinates, heading, and (future) store info
- **Visual status indicators** - RGB LED shows GPS lock and movement status
- **User interaction** - Push buttons to switch display modes

**Current Status:** The hardware accurately displays your position and heading. Future software updates will integrate a store location database/API to show distance and direction to the nearest Jolt retailer.

### Key Features

| Feature | Description |
|---------|-------------|
| GPS Tracking | NEO-6M module tracks your current location |
| Digital Compass | QMC5883L helps you walk in the right direction |
| OLED Display | Shows coordinates, heading, and store info (TBD) |
| Status LED | RGB indicator for GPS fix and movement |
| Distance Tracking | See how far you've walked to get your Jolt |
| Non-blocking Code | Smooth, responsive operation |

### Use Cases

- ⚡ Finding nearby Jolt energy drink stores
- 🚶 Walking navigation with compass guidance
- 🗺️ General GPS coordinate display
- 📚 Learning embedded systems and PCB design
- 🔧 First KiCad project portfolio piece

---

## 🔧 Hardware

### Components List

| Component | Part Number | Quantity | Description |
|-----------|-------------|----------|-------------|
| ESP32 DevKit V1 | ESP-WROOM-32 | 1 | Main microcontroller (3.3V logic) |
| GPS Module | NEO-6M | 1 | UART GPS receiver with antenna |
| Magnetometer | QMC5883L/HMC5883L | 1 | I2C digital compass |
| OLED Display | SSD1306 | 1 | 0.96" 128x64 I2C display |
| RGB LED | Common Cathode | 1 | Status indicator |
| Resistors | 330Ω | 3 | LED current limiting |
| Resistors | 4.7kΩ | 2 | I2C pull-ups |
| Push Buttons | 6mm Tactile | 2 | User input |
| Capacitors | 100nF | 3 | Decoupling |
| Capacitor | 10µF | 1 | Power filtering |

### Pin Mapping

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 DevKit V1                          │
├─────────────────────────────────────────────────────────────┤
│  GPIO16 (RX2) ◄──── GPS TX (NEO-6M)                         │
│  GPIO17 (TX2) ────► GPS RX (NEO-6M)                         │
│                                                             │
│  GPIO21 (SDA) ◄───► Magnetometer SDA + OLED SDA             │
│  GPIO22 (SCL) ◄───► Magnetometer SCL + OLED SCL             │
│                                                             │
│  GPIO25 ──────────► RGB LED Red (via 330Ω)                  │
│  GPIO26 ──────────► RGB LED Green (via 330Ω)                │
│  GPIO27 ──────────► RGB LED Blue (via 330Ω)                 │
│                                                             │
│  GPIO32 ◄────────── Button 1 (Mode) ──── GND                │
│  GPIO33 ◄────────── Button 2 (Calibrate) ── GND             │
│                                                             │
│  3.3V ───────────► VCC (GPS, Magnetometer, OLED)            │
│  GND ────────────► GND (All components)                     │
└─────────────────────────────────────────────────────────────┘
```

### Communication Protocols

| Protocol | Pins | Devices | Speed |
|----------|------|---------|-------|
| UART2 | GPIO16/17 | NEO-6M GPS | 9600 baud |
| I2C | GPIO21/22 | QMC5883L, SSD1306 | 400kHz |
| GPIO | 25, 26, 27 | RGB LED | Digital |
| GPIO | 32, 33 | Push Buttons | Digital (Pull-up) |

### Power Requirements

- **Input Voltage:** 5V via USB or 3.3V regulated
- **Current Draw:** ~150mA typical (GPS active)
- **Logic Level:** 3.3V (ESP32 native)

---

## 📐 Design Files

### Schematic
📄 [View Schematic PDF](KiCad_Assets/CircuitJoltLocatorSchematics.pdf)

### PCB 3D Render
![PCB 3D View](KiCad_Assets/Circuit3D.png)

### PCB Layers
| Layer | File |
|-------|------|
| Front Copper | [F_Cu.pdf](KiCad_Assets/PCB/Circuit-F_Cu.pdf) |
| Back Copper | [B_Cu.pdf](KiCad_Assets/PCB/Circuit-B_Cu.pdf) |
| Front Silkscreen | [F_Silkscreen.pdf](KiCad_Assets/PCB/Circuit-F_Silkscreen.pdf) |
| Back Silkscreen | [B_Silkscreen.pdf](KiCad_Assets/PCB/Circuit-B_Silkscreen.pdf) |
| Edge Cuts | [Edge_Cuts.pdf](KiCad_Assets/PCB/Circuit-Edge_Cuts.pdf) |

> **Designed in KiCad 9.0.6**

---

## 💻 Software

### Required Libraries

Install these libraries via Arduino IDE Library Manager:

| Library | Author | Purpose |
|---------|--------|---------|
| TinyGPSPlus | Mikal Hart | GPS NMEA parsing |
| Adafruit SSD1306 | Adafruit | OLED display driver |
| Adafruit GFX | Adafruit | Graphics primitives |
| QMC5883LCompass | MPrograms | Magnetometer driver |

### Installation Steps

1. **Install Arduino IDE** (2.0+ recommended)

2. **Add ESP32 Board Support:**
   - Go to `File` → `Preferences`
   - Add to Board Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools` → `Board` → `Boards Manager`
   - Search "ESP32" and install

3. **Install Libraries:**
   - Go to `Sketch` → `Include Library` → `Manage Libraries`
   - Search and install each library listed above

4. **Upload Code:**
   - Select Board: `ESP32 Dev Module`
   - Select Port: Your COM port
   - Click Upload

### Serial Monitor

- **Baud Rate:** 115200
- **Line Ending:** Newline
- Shows initialization status and GPS data updates

---

## 🛠️ Assembly

### Breadboard Wiring

```
                    ┌─────────────────┐
                    │   ESP32 DevKit  │
                    │                 │
    NEO-6M GPS      │     GPIO16 ◄────┼──── TX
    ┌────────┐      │     GPIO17 ────►┼──── RX
    │   TX ──┼──────┤                 │
    │   RX ──┼──────┤                 │
    │  VCC ──┼──────┤     3.3V        │
    │  GND ──┼──────┤     GND         │
    └────────┘      │                 │
                    │                 │
    QMC5883L        │     GPIO21 ◄───►┼──── SDA (+ 4.7kΩ to 3.3V)
    ┌────────┐      │     GPIO22 ◄───►┼──── SCL (+ 4.7kΩ to 3.3V)
    │  SDA ──┼──────┤                 │
    │  SCL ──┼──────┤                 │
    │  VCC ──┼──────┤     3.3V        │
    │  GND ──┼──────┤     GND         │
    └────────┘      │                 │
                    │                 │
    SSD1306 OLED    │     (Shared I2C bus)
    ┌────────┐      │                 │
    │  SDA ──┼──────┤     GPIO21      │
    │  SCL ──┼──────┤     GPIO22      │
    │  VCC ──┼──────┤     3.3V        │
    │  GND ──┼──────┤     GND         │
    └────────┘      │                 │
                    │                 │
    RGB LED         │     GPIO25 ────►┼──── R (330Ω)
    (Common         │     GPIO26 ────►┼──── G (330Ω)
     Cathode)       │     GPIO27 ────►┼──── B (330Ω)
                    │     GND ────────┼──── Cathode
                    │                 │
    Buttons         │     GPIO32 ◄────┼──── BTN1 ── GND
                    │     GPIO33 ◄────┼──── BTN2 ── GND
                    └─────────────────┘
```

### Step-by-Step Connection Guide

1. **Power Rails:** Connect 3.3V and GND rails on breadboard
2. **GPS Module:** Connect TX→GPIO16, RX→GPIO17, power to 3.3V
3. **I2C Bus:** Add 4.7kΩ pull-ups on SDA and SCL lines
4. **Magnetometer:** Connect to I2C bus (address 0x0D)
5. **OLED:** Connect to same I2C bus (address 0x3C)
6. **RGB LED:** Connect through 330Ω resistors to GPIO25/26/27
7. **Buttons:** Connect between GPIO32/33 and GND

### Testing Procedure

1. Upload code and open Serial Monitor (115200 baud)
2. Verify "Initialization complete!" message
3. Check each peripheral status in serial output
4. Take device outdoors for GPS fix (may take 1-5 minutes)
5. Test button functions and LED colors

---

## 🚀 Usage

### Display Modes

Press **Button 1** to cycle through display modes:

| Mode | Display Content |
|------|-----------------|
| GPS Info | Your location, heading, speed - ready to find Jolt! |
| Compass Only | Large compass rose to guide your walk to the store |
| System Info | Uptime, GPS status, distance traveled, store count (TBD) |

### Button Functions

| Button | Action |
|--------|--------|
| Button 1 (GPIO32) | Cycle display modes |
| Button 2 (GPIO33) | Start/stop compass calibration |

### LED Status Indicators

| Color | Meaning |
|-------|---------|
| 🔴 Red | No GPS fix - can't locate you yet |
| 🟡 Yellow | GPS locked, but you're standing still |
| 🟢 Green | GPS locked and you're on the move - go get that Jolt! |
| 🔵 Blue (blinking) | Compass calibration mode |

### Compass Calibration

1. Press Button 2 to enter calibration mode
2. Slowly rotate device 360° in all orientations
3. Continue for 15 seconds (progress bar shown)
4. Calibration completes automatically

---

## 📚 Learning Journey

### What I Learned About PCB Design

This project taught me the complete hardware design workflow:

**Schematic Design:**
- Creating and organizing schematic sheets
- Proper use of power symbols and net labels
- Hierarchical design principles
- Running Electrical Rules Check (ERC)

**Component Selection:**
- Choosing appropriate footprints
- Understanding package types (SMD vs through-hole)
- Reading datasheets for pin configurations
- Matching schematic symbols to PCB footprints

**PCB Layout:**
- Component placement strategies
- Trace routing and width calculations
- Ground plane design
- Via placement and usage
- Design Rule Check (DRC)

### Challenges Faced

| Challenge | Solution |
|-----------|----------|
| ERC errors with power flags | Added PWR_FLAG symbols to power nets |
| I2C address conflicts | Verified unique addresses (0x0D, 0x3C) |
| Trace width for power | Calculated based on current requirements |
| Component clearance | Adjusted footprint spacing in DRC rules |
| 3D model alignment | Manually adjusted model offsets |

### KiCad Workflow Insights

1. **Start with schematic** - Get the logic right first
2. **Annotate early** - Assign reference designators before layout
3. **Check footprints** - Verify before starting PCB
4. **Route power first** - Then signals
5. **Use design rules** - Let DRC catch mistakes
6. **Generate Gerbers last** - After all checks pass

### Future Improvements

- [ ] **Store database integration** - API/database of Jolt retailer locations
- [ ] **Distance to nearest store** - Show how far to walk for your energy fix
- [ ] **Direction arrow** - Point toward the nearest Jolt store
- [ ] Add battery management (TP4056 + 18650) for portable use
- [ ] Include SD card for logging your Jolt-hunting adventures
- [ ] Add buzzer alert when approaching a store
- [ ] Design 3D-printed enclosure (energy drink themed!)
- [ ] Implement Bluetooth for phone app connectivity

---

## 💰 Bill of Materials

| Component | Qty | Reference | Specifications | Est. Cost |
|-----------|-----|-----------|----------------|-----------|
| ESP32 DevKit V1 | 1 | U1 | ESP-WROOM-32, 38 pins | $5.00 |
| NEO-6M GPS Module | 1 | U2 | With ceramic antenna | $8.00 |
| QMC5883L Module | 1 | U3 | 3-axis magnetometer, I2C | $2.50 |
| SSD1306 OLED | 1 | U4 | 0.96" 128x64, I2C, White | $4.00 |
| RGB LED 5mm | 1 | D1 | Common cathode, diffused | $0.20 |
| Resistor 330Ω | 3 | R1-R3 | 1/4W, 5% | $0.10 |
| Resistor 4.7kΩ | 2 | R4-R5 | 1/4W, 5% (I2C pull-ups) | $0.10 |
| Tactile Switch | 2 | SW1-SW2 | 6x6mm, 4-pin | $0.20 |
| Capacitor 100nF | 3 | C1-C3 | Ceramic, decoupling | $0.15 |
| Capacitor 10µF | 1 | C4 | Electrolytic, power filter | $0.10 |
| Pin Headers | 1 set | J1-J4 | 2.54mm pitch | $0.50 |
| Breadboard | 1 | - | 830 tie points (for prototyping) | $3.00 |
| Jumper Wires | 1 set | - | Male-to-male, various lengths | $2.00 |

### Total Estimated Cost: **~$26**

### Where to Buy

- **Amazon:** [ESP32 DevKit](https://amazon.com), [NEO-6M GPS](https://amazon.com)
- **Robu.in (India):** [Components](https://robu.in)
- **AliExpress:** Budget-friendly bulk options
- **LCSC/JLCPCB:** For PCB fabrication and SMD components

---

## 📊 Project Status

- [x] Schematic design complete
- [x] PCB layout complete
- [x] 3D model visualization
- [x] Arduino firmware written (GPS + compass working)
- [ ] Breadboard prototype testing
- [ ] PCB fabrication order
- [ ] PCB assembly
- [ ] Store location database/API integration
- [ ] Enclosure design
- [ ] Field testing (finding actual Jolt stores!)

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

You are free to:
- Use this design for personal or commercial projects
- Modify and adapt the design
- Share and distribute

---

## 🤝 Contact

**Arceus Labs**

- 🌐 GitHub: [@Arceus-Labs](https://github.com/Arceus-Labs)
- 📧 Open an issue for questions or suggestions
- 🔗 Project Link: [https://github.com/Arceus-Labs/Jolt-Locator](https://github.com/Arceus-Labs/Jolt-Locator)

**Open for collaboration and suggestions!**

If you build this project or have improvements to suggest, I'd love to hear about it. Feel free to:
- ⭐ Star this repo if you find it useful
- 🍴 Fork and submit pull requests
- 🐛 Report issues or bugs
- 💡 Suggest new features (especially store database ideas!)
- ⚡ Share your Jolt-hunting adventures

---

## 🙏 Acknowledgments

- **KiCad Community** - For excellent documentation and tutorials
- **Random Nerd Tutorials** - ESP32 guides that got me started
- **Adafruit** - For amazing libraries and learning resources
- **Open Source Hardware Association** - For promoting open hardware
- **YouTube Creators** - Phil's Lab, EEVblog, and others for PCB design tutorials

---

<p align="center">
  <i>Built with ❤️ and ⚡ caffeine</i><br>
  <i>First PCB design - everyone starts somewhere!</i><br>
  <i>Now go find your Jolt!</i>
</p>
