/*
 * Jolt Locator – ESP32 Energy Drink Store Locator
 * Find your nearest Jolt energy drink store with GPS and compass guidance!
 * 
 * Hardware:
 *   - ESP32 DevKit V1 (3.3V logic)
 *   - NEO-6M GPS Module (UART2) - tracks your current location
 *   - HMC5883L/QMC5883L Magnetometer (I2C) - guides you in the right direction
 *   - SSD1306 0.96" OLED 128x64 (I2C) - displays location and heading info
 *   - RGB LED (Common Cathode) - status indicator
 *   - 2x Push Buttons (Active-low) - user controls
 * 
 * Future: Integrate store location database/API to show distance and
 *         direction to nearest Jolt energy drink retailer.
 * 
 * Author: Arceus Labs
 * License: MIT
 * Repository: https://github.com/Arceus-Labs/Jolt-Locator
 */

// ==================== LIBRARIES ====================
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

// ==================== CONFIGURATION ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// GPS Pins
#define RXD2 16
#define TXD2 17

// RGB LED Pins
#define LED_RED 25
#define LED_GREEN 26
#define LED_BLUE 27

// Button Pins
#define BUTTON_SAVE 18    // Start Navigation
#define BUTTON_CLEAR 19   // Stop Navigation

// Target Location - COS Market Thapar University
#define SHOP_LAT 30.9042
#define SHOP_LON 76.3673
#define SHOP_NAME "COS Market"

// Arrival threshold (meters)
#define ARRIVAL_DISTANCE 20

// ==================== OBJECTS ====================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);

// ==================== GLOBAL VARIABLES ====================
bool navigationActive = false;
bool gpsLocked = false;
bool hasArrived = false;
unsigned long lastUpdate = 0;
unsigned long gpsChars = 0;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n╔════════════════════════════════╗");
  Serial.println("║   JOLT LOCATOR v1.0 FINAL     ║");
  Serial.println("║   Energy Drink Finder         ║");
  Serial.println("║   Production Ready            ║");
  Serial.println("╚════════════════════════════════╝\n");
  
  // Initialize GPS
  gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("[✓] GPS initialized");
  
  // Initialize I2C
  Wire.begin(21, 22);
  
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[✗] OLED initialization FAILED!");
    while(1);
  }
  Serial.println("[✓] OLED initialized");
  
  // Initialize Compass
  if(!compass.begin()) {
    Serial.println("[!] Compass not detected - navigation will work without it");
  } else {
    Serial.println("[✓] Compass initialized");
  }
  
  // Initialize RGB LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  Serial.println("[✓] RGB LED initialized");
  
  // RGB Startup Test
  Serial.println("[•] Testing RGB LED...");
  setColor(255, 0, 0); delay(200);
  setColor(0, 255, 0); delay(200);
  setColor(0, 0, 255); delay(200);
  setColor(0, 0, 0);
  Serial.println("[✓] RGB LED test complete");
  
  // Initialize Buttons
  pinMode(BUTTON_SAVE, INPUT_PULLUP);
  pinMode(BUTTON_CLEAR, INPUT_PULLUP);
  Serial.println("[✓] Buttons initialized");
  
  // Show Startup Screen
  showSplashScreen();
  
  Serial.println("\n[•] System ready!");
  Serial.println("[•] Target: " + String(SHOP_NAME));
  Serial.print("[•] Coordinates: ");
  Serial.print(SHOP_LAT, 6);
  Serial.print(", ");
  Serial.println(SHOP_LON, 6);
  Serial.println("\n[⏳] Waiting for GPS lock...\n");
}

// ==================== MAIN LOOP ====================
void loop() {
  // Feed GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    gpsChars++;
  }
  
  // Check for GPS lock
  if (gps.location.isValid() && !gpsLocked) {
    gpsLocked = true;
    onGPSLocked();
  }
  
  // Get compass heading
  float heading = getCompassHeading();
  
  // Handle button presses
  handleButtons();
  
  // Update display (throttled to 500ms)
  if (millis() - lastUpdate > 500) {
    updateDisplay(heading);
    lastUpdate = millis();
  }
  
  // Update LED indicator
  updateLED(heading);
  
  delay(100);
}

// ==================== GPS LOCK EVENT ====================
void onGPSLocked() {
  Serial.println("\n╔════════════════════════════════╗");
  Serial.println("║      GPS LOCK ACQUIRED!       ║");
  Serial.println("╚════════════════════════════════╝");
  Serial.print("Latitude:    ");
  Serial.println(gps.location.lat(), 6);
  Serial.print("Longitude:   ");
  Serial.println(gps.location.lng(), 6);
  Serial.print("Satellites:  ");
  Serial.println(gps.satellites.value());
  Serial.print("Altitude:    ");
  Serial.print(gps.altitude.meters(), 1);
  Serial.println(" m");
  Serial.print("Speed:       ");
  Serial.print(gps.speed.kmph(), 1);
  Serial.println(" km/h");
  
  // Calculate distance to shop
  float dist = calculateDistance();
  Serial.print("Distance to ");
  Serial.print(SHOP_NAME);
  Serial.print(": ");
  if (dist < 1000) {
    Serial.print((int)dist);
    Serial.println(" m");
  } else {
    Serial.print(dist/1000.0, 2);
    Serial.println(" km");
  }
  Serial.println("\n[•] Press SAVE button to start navigation\n");
  
  // Green flash
  for(int i=0; i<3; i++) {
    setColor(0, 255, 0);
    delay(200);
    setColor(0, 0, 0);
    delay(200);
  }
}

// ==================== BUTTON HANDLING ====================
void handleButtons() {
  static unsigned long lastPress = 0;
  
  // SAVE Button - Start Navigation
  if (digitalRead(BUTTON_SAVE) == LOW && millis() - lastPress > 300) {
    if (gpsLocked) {
      navigationActive = true;
      hasArrived = false;
      
      Serial.println("\n[▶] NAVIGATION STARTED");
      Serial.print("[→] Target: ");
      Serial.println(SHOP_NAME);
      
      float dist = calculateDistance();
      Serial.print("[📍] Distance: ");
      if (dist < 1000) {
        Serial.print((int)dist);
        Serial.println(" m");
      } else {
        Serial.print(dist/1000.0, 2);
        Serial.println(" km");
      }
      
      double course = calculateCourse();
      Serial.print("[🧭] Direction: ");
      Serial.print(getDirectionName(course));
      Serial.print(" (");
      Serial.print((int)course);
      Serial.println("°)\n");
      
      // Green flash
      setColor(0, 255, 0);
      delay(300);
      setColor(0, 0, 0);
      
    } else {
      Serial.println("[✗] GPS not locked! Cannot start navigation.");
      // Red flash
      setColor(255, 0, 0);
      delay(300);
      setColor(0, 0, 0);
    }
    lastPress = millis();
  }
  
  // CLEAR Button - Stop Navigation
  if (digitalRead(BUTTON_CLEAR) == LOW && millis() - lastPress > 300) {
    if (navigationActive) {
      navigationActive = false;
      Serial.println("\n[■] NAVIGATION STOPPED\n");
      // Red flash
      setColor(255, 0, 0);
      delay(300);
      setColor(0, 0, 0);
    }
    lastPress = millis();
  }
}

// ==================== DISPLAY UPDATE ====================
void updateDisplay(float heading) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  // Header
  display.println("=== JOLT LOCATOR ===");
  display.println("");
  
  // GPS Status
  if (gpsLocked) {
    display.print("GPS: LOCK (");
    display.print(gps.satellites.value());
    display.println(" sats)");
    
    // Current position
    display.print(gps.location.lat(), 4);
    display.print(",");
    display.println(gps.location.lng(), 4);
    
  } else {
    display.println("GPS: SEARCHING...");
    display.print("Chars: ");
    display.println(gpsChars);
    
    if (gpsChars == 0) {
      display.println("NO DATA!");
      display.println("Check wiring");
    } else {
      display.println("Stay outside");
      display.println("Wait 5-15 mins");
      if (gps.satellites.isValid()) {
        display.print("Sats: ");
        display.println(gps.satellites.value());
      }
    }
  }
  
  display.println("");
  
  // Navigation Info
  if (navigationActive && gpsLocked) {
    display.print("Target: ");
    display.println(SHOP_NAME);
    
    float distance = calculateDistance();
    double course = calculateCourse();
    
    // Distance
    display.print("Dist: ");
    if (distance < 1000) {
      display.print((int)distance);
      display.println(" m");
    } else {
      display.print(distance/1000.0, 2);
      display.println(" km");
    }
    
    // Direction to target
    display.print("Go: ");
    display.print(getDirectionName(course));
    display.print(" (");
    display.print((int)course);
    display.println("*)");
    
    // Current heading
    display.print("Heading: ");
    display.print((int)heading);
    display.print("* ");
    display.println(getDirectionName(heading));
    
    // Arrival check
    if (distance < ARRIVAL_DISTANCE) {
      if (!hasArrived) {
        hasArrived = true;
        Serial.println("\n╔════════════════════════════════╗");
        Serial.println("║    DESTINATION REACHED! 🎉    ║");
        Serial.println("╚════════════════════════════════╝\n");
      }
      display.println("");
      display.setTextSize(2);
      display.println("ARRIVED!");
    }
    
  } else if (gpsLocked) {
    display.println("Ready to navigate!");
    display.println("");
    display.println("Press SAVE to go");
    display.print("to ");
    display.println(SHOP_NAME);
    display.println("");
    display.print("Head: ");
    display.print((int)heading);
    display.print("* ");
    display.println(getDirectionName(heading));
  }
  
  display.display();
}

// ==================== LED INDICATOR ====================
void updateLED(float heading) {
  // Not navigating - LED off
  if (!navigationActive) {
    if (!gpsLocked && gpsChars > 100) {
      // Blink blue while searching
      static unsigned long lastBlink = 0;
      static bool state = false;
      if (millis() - lastBlink > 1000) {
        state = !state;
        setColor(0, 0, state ? 255 : 0);
        lastBlink = millis();
      }
    } else {
      setColor(0, 0, 0);
    }
    return;
  }
  
  // Navigating - check if locked
  if (!gpsLocked) {
    setColor(0, 0, 0);
    return;
  }
  
  float distance = calculateDistance();
  double course = calculateCourse();
  
  // Arrived - solid green
  if (distance < ARRIVAL_DISTANCE) {
    setColor(0, 255, 0);
    return;
  }
  
  // Direction indicator
  int diff = abs((int)heading - (int)course);
  if (diff > 180) diff = 360 - diff;
  
  if (diff < 30) {
    setColor(0, 255, 0);      // Green - on track
  } else if (diff < 60) {
    setColor(255, 255, 0);    // Yellow - slight turn
  } else if (diff < 120) {
    setColor(255, 128, 0);    // Orange - big turn
  } else {
    setColor(255, 0, 0);      // Red - wrong way
  }
}

// ==================== HELPER FUNCTIONS ====================

void showSplashScreen() {
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(15, 10);
  display.println("JOLT");
  display.setTextSize(2);
  display.setCursor(0, 35);
  display.println("LOCATOR");
  display.setTextSize(1);
  display.setCursor(25, 56);
  display.println("v1.0 Final");
  display.display();
  
  setColor(0, 0, 255);
  delay(2500);
  setColor(0, 0, 0);
}

float getCompassHeading() {
  sensors_event_t event;
  compass.getEvent(&event);
  
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  heading = heading * 180.0 / PI;
  
  if (heading < 0) {
    heading += 360.0;
  }
  
  return heading;
}

float calculateDistance() {
  if (!gpsLocked) return 0;
  
  return TinyGPSPlus::distanceBetween(
    gps.location.lat(), gps.location.lng(),
    SHOP_LAT, SHOP_LON
  );
}

double calculateCourse() {
  if (!gpsLocked) return 0;
  
  return TinyGPSPlus::courseTo(
    gps.location.lat(), gps.location.lng(),
    SHOP_LAT, SHOP_LON
  );
}

String getDirectionName(double degrees) {
  int angle = (int)degrees;
  
  if (angle < 22 || angle >= 338) return "N";
  if (angle < 67) return "NE";
  if (angle < 112) return "E";
  if (angle < 157) return "SE";
  if (angle < 202) return "S";
  if (angle < 247) return "SW";
  if (angle < 292) return "W";
  return "NW";
}

void setColor(int red, int green, int blue) {
  analogWrite(LED_RED, red);
  analogWrite(LED_GREEN, green);
  analogWrite(LED_BLUE, blue);
}
