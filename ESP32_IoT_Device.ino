/*
 * ESP32 IoT Navigation Device
 * GPS Tracker with Digital Compass and OLED Display
 * 
 * Hardware:
 *   - ESP32 DevKit V1 (3.3V logic)
 *   - NEO-6M GPS Module (UART2)
 *   - HMC5883L/QMC5883L Magnetometer (I2C)
 *   - SSD1306 0.96" OLED 128x64 (I2C)
 *   - RGB LED (Common Cathode)
 *   - 2x Push Buttons (Active-low)
 * 
 * Author: Arceus Labs
 * License: MIT
 * Repository: https://github.com/Arceus-Labs/Jolt-Locator
 */

// ==================== LIBRARIES ====================
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <QMC5883LCompass.h>

// ==================== PIN DEFINITIONS ====================
// GPS Module (NEO-6M) - UART2
#define GPS_RX_PIN      16    // ESP32 RX2 ← GPS TX
#define GPS_TX_PIN      17    // ESP32 TX2 → GPS RX
#define GPS_BAUD        9600

// I2C Bus (Shared: Magnetometer + OLED)
#define I2C_SDA         21
#define I2C_SCL         22

// OLED Display (SSD1306)
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1    // No reset pin
#define OLED_ADDRESS    0x3C

// RGB LED (Common Cathode, Active High)
#define LED_RED         25
#define LED_GREEN       26
#define LED_BLUE        27

// Push Buttons (Active Low with Internal Pull-ups)
#define BTN_MODE        32    // Button 1: Toggle display mode
#define BTN_CALIBRATE   33    // Button 2: Compass calibration

// ==================== CONSTANTS ====================
#define DEBOUNCE_DELAY      50      // Button debounce (ms)
#define GPS_UPDATE_INTERVAL 1000    // GPS read interval (ms)
#define DISPLAY_INTERVAL    200     // Display refresh (ms)
#define COMPASS_INTERVAL    100     // Compass read interval (ms)
#define LED_BLINK_INTERVAL  500     // LED blink rate (ms)
#define CALIBRATION_TIME    15000   // Calibration duration (ms)
#define SPEED_THRESHOLD     0.5     // Speed threshold for "moving" (km/h)
#define COMPASS_FILTER      0.2     // Low-pass filter coefficient (0-1)

// Display modes
enum DisplayMode {
  MODE_GPS_INFO,
  MODE_COMPASS_ONLY,
  MODE_SYSTEM_INFO,
  MODE_COUNT
};

// LED colors
enum LedColor {
  COLOR_OFF,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW
};

// ==================== GLOBAL OBJECTS ====================
TinyGPSPlus gps;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
QMC5883LCompass compass;

// ==================== STATE VARIABLES ====================
// Display
DisplayMode currentMode = MODE_GPS_INFO;
bool displayInitialized = false;

// GPS Data
double latitude = 0.0;
double longitude = 0.0;
double altitude = 0.0;
double speed_kmh = 0.0;
int satellites = 0;
bool gpsValid = false;
int gpsHour = 0, gpsMinute = 0, gpsSecond = 0;
bool timeValid = false;

// Distance tracking
double totalDistance = 0.0;
double lastLat = 0.0;
double lastLon = 0.0;
bool hasLastPosition = false;

// Compass
int rawHeading = 0;
float filteredHeading = 0.0;
bool compassInitialized = false;
bool calibrationMode = false;
unsigned long calibrationStartTime = 0;

// Buttons
bool btn1LastState = HIGH;
bool btn2LastState = HIGH;
unsigned long btn1LastDebounce = 0;
unsigned long btn2LastDebounce = 0;

// Timing (non-blocking)
unsigned long lastGpsUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastCompassUpdate = 0;
unsigned long lastLedBlink = 0;
bool ledBlinkState = false;

// System info
unsigned long startTime = 0;

// ==================== FUNCTION PROTOTYPES ====================
void initGPS();
void initCompass();
void initDisplay();
void initLED();
void initButtons();

void readGPS();
void readCompass();
void updateDisplay();
void updateLED();
void handleButtons();

void setLedColor(LedColor color);
const char* getCardinalDirection(int heading);
String formatCoordinate(double coord, bool isLat);
String formatTime(int h, int m, int s);
String formatUptime(unsigned long ms);

void displayGPSInfo();
void displayCompassOnly();
void displaySystemInfo();
void displayCalibration();

void startCalibration();
void stopCalibration();

// ==================== SETUP ====================
void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(100);
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("  ESP32 IoT Navigation Device");
  Serial.println("  Jolt Locator - Arceus Labs");
  Serial.println("========================================");
  Serial.println();
  
  startTime = millis();
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("[I2C] Bus initialized on SDA:" + String(I2C_SDA) + " SCL:" + String(I2C_SCL));
  
  // Initialize peripherals
  initLED();
  initButtons();
  initDisplay();
  initCompass();
  initGPS();
  
  Serial.println();
  Serial.println("[SYSTEM] Initialization complete!");
  Serial.println("[SYSTEM] Waiting for GPS fix...");
  Serial.println();
}

// ==================== MAIN LOOP ====================
void loop() {
  unsigned long currentMillis = millis();
  
  // Handle button inputs (always responsive)
  handleButtons();
  
  // Read GPS data
  if (currentMillis - lastGpsUpdate >= GPS_UPDATE_INTERVAL) {
    lastGpsUpdate = currentMillis;
    readGPS();
  }
  
  // Read compass data
  if (currentMillis - lastCompassUpdate >= COMPASS_INTERVAL) {
    lastCompassUpdate = currentMillis;
    readCompass();
  }
  
  // Update display
  if (currentMillis - lastDisplayUpdate >= DISPLAY_INTERVAL) {
    lastDisplayUpdate = currentMillis;
    updateDisplay();
  }
  
  // Update LED status
  updateLED();
  
  // Check calibration timeout
  if (calibrationMode && (currentMillis - calibrationStartTime >= CALIBRATION_TIME)) {
    stopCalibration();
  }
  
  // Feed GPS parser continuously
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }
}


// ==================== INITIALIZATION FUNCTIONS ====================

void initGPS() {
  Serial.print("[GPS] Initializing on UART2... ");
  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(100);
  Serial.println("OK");
  Serial.println("[GPS] Baud: " + String(GPS_BAUD) + " | RX:" + String(GPS_RX_PIN) + " TX:" + String(GPS_TX_PIN));
}

void initCompass() {
  Serial.print("[COMPASS] Initializing QMC5883L... ");
  
  compass.init();
  compass.setSmoothing(10, true);  // Enable smoothing with 10 samples
  
  // Test read to verify connection
  compass.read();
  int testHeading = compass.getAzimuth();
  
  if (testHeading >= 0 && testHeading <= 360) {
    compassInitialized = true;
    filteredHeading = testHeading;
    Serial.println("OK");
    Serial.println("[COMPASS] Initial heading: " + String(testHeading) + "°");
  } else {
    compassInitialized = false;
    Serial.println("FAILED");
    Serial.println("[COMPASS] Warning: Sensor not responding correctly");
  }
}

void initDisplay() {
  Serial.print("[OLED] Initializing SSD1306... ");
  
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    displayInitialized = true;
    Serial.println("OK");
    
    // Show splash screen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 10);
    display.println("JOLT LOCATOR");
    display.setCursor(30, 25);
    display.println("Arceus Labs");
    display.setCursor(15, 45);
    display.println("Initializing...");
    display.display();
    delay(1500);
  } else {
    displayInitialized = false;
    Serial.println("FAILED");
    Serial.println("[OLED] Error: Display not found at 0x" + String(OLED_ADDRESS, HEX));
  }
}

void initLED() {
  Serial.print("[LED] Initializing RGB LED... ");
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Test sequence
  setLedColor(COLOR_RED);
  delay(200);
  setLedColor(COLOR_GREEN);
  delay(200);
  setLedColor(COLOR_BLUE);
  delay(200);
  setLedColor(COLOR_OFF);
  
  Serial.println("OK");
  Serial.println("[LED] Pins - R:" + String(LED_RED) + " G:" + String(LED_GREEN) + " B:" + String(LED_BLUE));
}

void initButtons() {
  Serial.print("[BUTTONS] Initializing... ");
  
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_CALIBRATE, INPUT_PULLUP);
  
  Serial.println("OK");
  Serial.println("[BUTTONS] Mode:" + String(BTN_MODE) + " Calibrate:" + String(BTN_CALIBRATE));
}

// ==================== SENSOR READING FUNCTIONS ====================

void readGPS() {
  // Update GPS data if valid
  if (gps.location.isValid()) {
    double newLat = gps.location.lat();
    double newLon = gps.location.lng();
    
    // Calculate distance traveled
    if (hasLastPosition && gpsValid) {
      double dist = TinyGPSPlus::distanceBetween(lastLat, lastLon, newLat, newLon);
      if (dist > 1.0 && dist < 1000.0) {  // Filter noise (1m - 1km)
        totalDistance += dist;
      }
    }
    
    lastLat = latitude;
    lastLon = longitude;
    hasLastPosition = gpsValid;
    
    latitude = newLat;
    longitude = newLon;
    gpsValid = true;
  }
  
  if (gps.altitude.isValid()) {
    altitude = gps.altitude.meters();
  }
  
  if (gps.speed.isValid()) {
    speed_kmh = gps.speed.kmph();
  }
  
  if (gps.satellites.isValid()) {
    satellites = gps.satellites.value();
  }
  
  if (gps.time.isValid()) {
    gpsHour = gps.time.hour();
    gpsMinute = gps.time.minute();
    gpsSecond = gps.time.second();
    timeValid = true;
  }
  
  // Debug output
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug >= 5000) {
    lastDebug = millis();
    Serial.println("[GPS] Sats:" + String(satellites) + 
                   " | Lat:" + String(latitude, 6) + 
                   " | Lon:" + String(longitude, 6) +
                   " | Alt:" + String(altitude, 1) + "m" +
                   " | Spd:" + String(speed_kmh, 1) + "km/h");
  }
}

void readCompass() {
  if (!compassInitialized) return;
  
  compass.read();
  rawHeading = compass.getAzimuth();
  
  // Validate reading
  if (rawHeading < 0 || rawHeading > 360) {
    return;
  }
  
  // Apply low-pass filter for smooth readings
  // Handle wrap-around at 0/360 degrees
  float diff = rawHeading - filteredHeading;
  if (diff > 180) diff -= 360;
  if (diff < -180) diff += 360;
  
  filteredHeading += COMPASS_FILTER * diff;
  
  // Normalize to 0-360
  if (filteredHeading < 0) filteredHeading += 360;
  if (filteredHeading >= 360) filteredHeading -= 360;
}


// ==================== DISPLAY FUNCTIONS ====================

void updateDisplay() {
  if (!displayInitialized) return;
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  if (calibrationMode) {
    displayCalibration();
  } else {
    switch (currentMode) {
      case MODE_GPS_INFO:
        displayGPSInfo();
        break;
      case MODE_COMPASS_ONLY:
        displayCompassOnly();
        break;
      case MODE_SYSTEM_INFO:
        displaySystemInfo();
        break;
      default:
        displayGPSInfo();
    }
  }
  
  display.display();
}

void displayGPSInfo() {
  display.setTextSize(1);
  
  // Line 1: Coordinates
  display.setCursor(0, 0);
  if (gpsValid) {
    display.print("Lat:");
    display.println(String(latitude, 5));
    display.print("Lon:");
    display.println(String(longitude, 5));
  } else {
    display.println("Lat: ---.-----");
    display.println("Lon: ---.-----");
  }
  
  // Line 3: Satellites + Time
  display.setCursor(0, 24);
  display.print("Sats: ");
  display.print(satellites);
  if (timeValid) {
    display.print("  ");
    display.print(formatTime(gpsHour, gpsMinute, gpsSecond));
  }
  
  // Line 4: Compass heading
  display.setCursor(0, 36);
  display.print("Hdg: ");
  if (compassInitialized) {
    display.print((int)filteredHeading);
    display.print((char)247);  // Degree symbol
    display.print(" ");
    display.print(getCardinalDirection((int)filteredHeading));
  } else {
    display.print("N/A");
  }
  
  // Line 5: Speed or Altitude
  display.setCursor(0, 48);
  if (speed_kmh > SPEED_THRESHOLD) {
    display.print("Spd: ");
    display.print(speed_kmh, 1);
    display.print(" km/h");
  } else {
    display.print("Alt: ");
    display.print(altitude, 0);
    display.print(" m");
  }
  
  // Status indicator
  display.setCursor(100, 0);
  if (satellites >= 4) {
    display.print("FIX");
  } else {
    display.print("...");
  }
}

void displayCompassOnly() {
  // Large compass display
  display.setTextSize(1);
  display.setCursor(35, 0);
  display.println("COMPASS");
  
  // Draw compass circle
  int centerX = 64;
  int centerY = 38;
  int radius = 22;
  display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);
  
  // Draw cardinal points
  display.setCursor(centerX - 3, centerY - radius - 8);
  display.print("N");
  display.setCursor(centerX - 3, centerY + radius + 2);
  display.print("S");
  display.setCursor(centerX + radius + 3, centerY - 3);
  display.print("E");
  display.setCursor(centerX - radius - 9, centerY - 3);
  display.print("W");
  
  // Draw heading needle
  if (compassInitialized) {
    float headingRad = (filteredHeading - 90) * PI / 180.0;  // Adjust for display orientation
    int needleX = centerX + (radius - 5) * cos(headingRad);
    int needleY = centerY + (radius - 5) * sin(headingRad);
    display.drawLine(centerX, centerY, needleX, needleY, SSD1306_WHITE);
    display.fillCircle(needleX, needleY, 2, SSD1306_WHITE);
  }
  
  // Display heading value
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("Heading: ");
  if (compassInitialized) {
    display.print((int)filteredHeading);
    display.print((char)247);
    display.print(" ");
    display.print(getCardinalDirection((int)filteredHeading));
  } else {
    display.print("N/A");
  }
}

void displaySystemInfo() {
  display.setTextSize(1);
  
  display.setCursor(0, 0);
  display.println("=== SYSTEM INFO ===");
  
  display.setCursor(0, 12);
  display.print("Uptime: ");
  display.println(formatUptime(millis() - startTime));
  
  display.setCursor(0, 24);
  display.print("GPS: ");
  display.println(gpsValid ? "Connected" : "Searching");
  
  display.setCursor(0, 36);
  display.print("Compass: ");
  display.println(compassInitialized ? "OK" : "Error");
  
  display.setCursor(0, 48);
  display.print("Distance: ");
  if (totalDistance < 1000) {
    display.print(totalDistance, 0);
    display.println(" m");
  } else {
    display.print(totalDistance / 1000.0, 2);
    display.println(" km");
  }
  
  display.setCursor(0, 56);
  display.print("Mode: ");
  display.print(currentMode + 1);
  display.print("/");
  display.print(MODE_COUNT);
}

void displayCalibration() {
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.println("COMPASS CALIBRATION");
  
  display.setCursor(0, 16);
  display.println("Rotate device slowly");
  display.println("360 degrees in all");
  display.println("orientations");
  
  // Progress bar
  unsigned long elapsed = millis() - calibrationStartTime;
  int progress = map(elapsed, 0, CALIBRATION_TIME, 0, 100);
  progress = constrain(progress, 0, 100);
  
  display.setCursor(0, 48);
  display.print("Progress: ");
  display.print(progress);
  display.println("%");
  
  // Draw progress bar
  display.drawRect(0, 56, 128, 8, SSD1306_WHITE);
  display.fillRect(2, 58, map(progress, 0, 100, 0, 124), 4, SSD1306_WHITE);
}


// ==================== LED FUNCTIONS ====================

void updateLED() {
  unsigned long currentMillis = millis();
  
  if (calibrationMode) {
    // Blink blue during calibration
    if (currentMillis - lastLedBlink >= LED_BLINK_INTERVAL) {
      lastLedBlink = currentMillis;
      ledBlinkState = !ledBlinkState;
      if (ledBlinkState) {
        setLedColor(COLOR_BLUE);
      } else {
        setLedColor(COLOR_OFF);
      }
    }
    return;
  }
  
  // Normal operation LED status
  if (satellites < 4) {
    // RED: No GPS fix
    setLedColor(COLOR_RED);
  } else if (speed_kmh > SPEED_THRESHOLD) {
    // GREEN: GPS fix and moving
    setLedColor(COLOR_GREEN);
  } else {
    // YELLOW: GPS fix but stationary
    setLedColor(COLOR_YELLOW);
  }
}

void setLedColor(LedColor color) {
  switch (color) {
    case COLOR_OFF:
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, LOW);
      break;
    case COLOR_RED:
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, LOW);
      break;
    case COLOR_GREEN:
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE, LOW);
      break;
    case COLOR_BLUE:
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, HIGH);
      break;
    case COLOR_YELLOW:
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE, LOW);
      break;
  }
}

// ==================== BUTTON HANDLING ====================

void handleButtons() {
  unsigned long currentMillis = millis();
  
  // Button 1: Toggle display mode
  bool btn1State = digitalRead(BTN_MODE);
  if (btn1State != btn1LastState) {
    btn1LastDebounce = currentMillis;
  }
  if ((currentMillis - btn1LastDebounce) > DEBOUNCE_DELAY) {
    if (btn1State == LOW && btn1LastState == HIGH) {
      // Button pressed
      if (!calibrationMode) {
        currentMode = (DisplayMode)((currentMode + 1) % MODE_COUNT);
        Serial.println("[BUTTON] Mode changed to: " + String(currentMode));
      }
    }
  }
  btn1LastState = btn1State;
  
  // Button 2: Calibration mode
  bool btn2State = digitalRead(BTN_CALIBRATE);
  if (btn2State != btn2LastState) {
    btn2LastDebounce = currentMillis;
  }
  if ((currentMillis - btn2LastDebounce) > DEBOUNCE_DELAY) {
    if (btn2State == LOW && btn2LastState == HIGH) {
      // Button pressed
      if (calibrationMode) {
        stopCalibration();
      } else {
        startCalibration();
      }
    }
  }
  btn2LastState = btn2State;
}

void startCalibration() {
  calibrationMode = true;
  calibrationStartTime = millis();
  compass.setCalibration(-32768, 32767, -32768, 32767, -32768, 32767);
  Serial.println("[COMPASS] Calibration started - rotate device 360 degrees");
}

void stopCalibration() {
  calibrationMode = false;
  Serial.println("[COMPASS] Calibration complete");
}

// ==================== UTILITY FUNCTIONS ====================

const char* getCardinalDirection(int heading) {
  // Normalize heading to 0-359
  heading = heading % 360;
  if (heading < 0) heading += 360;
  
  // 8-point compass rose
  if (heading >= 337 || heading < 23)  return "N";
  if (heading >= 23 && heading < 68)   return "NE";
  if (heading >= 68 && heading < 113)  return "E";
  if (heading >= 113 && heading < 158) return "SE";
  if (heading >= 158 && heading < 203) return "S";
  if (heading >= 203 && heading < 248) return "SW";
  if (heading >= 248 && heading < 293) return "W";
  if (heading >= 293 && heading < 337) return "NW";
  return "?";
}

String formatCoordinate(double coord, bool isLat) {
  char buffer[16];
  char dir;
  
  if (isLat) {
    dir = (coord >= 0) ? 'N' : 'S';
  } else {
    dir = (coord >= 0) ? 'E' : 'W';
  }
  
  coord = abs(coord);
  int degrees = (int)coord;
  double minutes = (coord - degrees) * 60.0;
  
  sprintf(buffer, "%d%c%.3f'%c", degrees, 247, minutes, dir);
  return String(buffer);
}

String formatTime(int h, int m, int s) {
  char buffer[10];
  sprintf(buffer, "%02d:%02d:%02d", h, m, s);
  return String(buffer);
}

String formatUptime(unsigned long ms) {
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  seconds %= 60;
  minutes %= 60;
  
  char buffer[12];
  sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(buffer);
}
