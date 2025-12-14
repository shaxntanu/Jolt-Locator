# ⚡ The Jolt Locator 🧭

> *When you NEED that Monster Energy and you need it NOW.*

A true-north, GPS-enabled compass built on the ESP32 that points directly to the nearest Monster Energy drink store. Because sometimes Google Maps just isn't aggressive enough.

## 🎯 What Does It Do?

The Jolt Locator is a digital compass with one mission: guide you to caffeinated glory. Instead of pointing north like some boring regular compass, it calculates the Great Circle Bearing to your target destination and tells you exactly which way to walk, run, or sprint.

**Features:**
- Real-time GPS positioning via NEO-6M module
- Magnetic heading from HMC5883L magnetometer
- Great Circle Bearing calculation (because the Earth is round, deal with it)
- Magnetic declination correction for true-north accuracy
- Distance-to-target calculation
- Visual direction indicators (↑ ↗ → ↘ ↓ ↙ ← ↖)

## 🔧 Hardware Requirements

| Component | Description |
|-----------|-------------|
| ESP32 Dev Board | The brain of the operation |
| NEO-6M GPS Module | Knows where you are |
| HMC5883L Magnetometer | Knows which way you're facing |
| Jumper Wires | The nervous system |
| Breadboard | Optional but recommended |
| Desperation for caffeine | Required |

## 📚 Required Libraries

Install these via Arduino Library Manager:

- **TinyGPS++** by Mikal Hart
- **Adafruit Unified Sensor** by Adafruit
- **Adafruit HMC5883 Unified** by Adafruit

## 🔌 Wiring Table

### GPS Module (NEO-6M)

| NEO-6M Pin | ESP32 Pin | Notes |
|------------|-----------|-------|
| VCC | 3.3V | ⚠️ Use 3.3V, not 5V! |
| GND | GND | Ground |
| TX | GPIO16 | GPS transmits to ESP32 RX |
| RX | GPIO17 | ESP32 transmits to GPS TX |

### Compass Module (HMC5883L)

| HMC5883L Pin | ESP32 Pin | Notes |
|--------------|-----------|-------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| SDA | GPIO21 | I2C Data |
| SCL | GPIO22 | I2C Clock |

## ⚙️ Configuration

### Setting Your Target

Open `JoltLocator.ino` and find these lines near the top:

```cpp
// ============================================================================
// TARGET DESTINATION - CHANGE THESE COORDINATES TO YOUR MONSTER ENERGY STORE!
// ============================================================================
const double TARGET_LAT = 40.748440;   // Latitude of destination
const double TARGET_LON = -73.985664;  // Longitude of destination
// ============================================================================
```

Replace with your local Monster Energy dealer's coordinates. Google Maps is your friend.

### Magnetic Declination

For accurate heading, set your local magnetic declination:

```cpp
float MAGNETIC_DECLINATION = -12.5;  // CHANGE THIS FOR YOUR LOCATION!
```

Find your declination at: https://www.ngdc.noaa.gov/geomag/calculators/magcalc.shtml

## 🚀 Usage

1. Upload the sketch to your ESP32
2. Open Serial Monitor at 115200 baud
3. Wait for GPS fix (may take 30-60 seconds outdoors)
4. Follow the pointer angle to your destination
5. Acquire Monster Energy
6. Repeat

## 📟 Serial Output Example

```
========================================
📍 CURRENT LOCATION:
   Lat: 40.712776
   Lon: -74.005974
   Satellites: 8

🎯 TARGET:
   Lat: 40.748440
   Lon: -73.985664
   Distance: 4.32 km

🧭 NAVIGATION:
   Magnetic Heading: 45.2°
   True Heading: 32.7°
   Target Bearing: 28.5°

⚡ POINTER ANGLE:
   >>> -4.2° <<<
   ↑ STRAIGHT AHEAD
========================================
```

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| No GPS data | Check TX/RX wiring (try swapping) |
| Compass not found | Verify I2C connections, check address |
| Heading is wrong | Calibrate compass, check declination |
| Still no Monster | Walk faster |

## 📜 License

MIT License - Do whatever you want, just stay caffeinated.

---

*Built with ⚡ and an unhealthy relationship with energy drinks.*
