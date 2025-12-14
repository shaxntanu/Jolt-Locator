/**
 * ⚡ The Jolt Locator 🧭
 * 
 * A GPS-enabled digital compass that points to the nearest Monster Energy store
 * (or any hardcoded destination coordinates).
 * 
 * Hardware:
 *   - ESP32 Development Board
 *   - NEO-6M GPS Module (Serial2: RX=GPIO16, TX=GPIO17)
 *   - HMC5883L Magnetometer (I2C: SDA=GPIO21, SCL=GPIO22)
 * 
 * Author: Jolt Locator Team
 * License: MIT
 */

#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

// ============================================================================
// TARGET DESTINATION - CHANGE THESE COORDINATES TO YOUR ENERGY DRINK STORE!
// ============================================================================
const double TARGET_LAT = 40.748440;   // Latitude of destination
const double TARGET_LON = -73.985664;  // Longitude of destination
// ============================================================================

// GPS Configuration
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD 9600

// Magnetic Declination Correction (in degrees)
// Find your local declination at: https://www.ngdc.noaa.gov/geomag/calculators/magcalc.shtml
// Positive = East declination, Negative = West declination
float MAGNETIC_DECLINATION = -12.5;  // CHANGE THIS FOR YOUR LOCATION!

// Serial Monitor baud rate
#define SERIAL_BAUD 115200

// Update interval (milliseconds)
#define UPDATE_INTERVAL 1000

// Objects
TinyGPSPlus gps;
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);

// Timing
unsigned long lastUpdate = 0;

// Current position
double currentLat = 0.0;
double currentLon = 0.0;
bool gpsValid = false;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(SERIAL_BAUD);
  while (!Serial) { delay(10); }
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("   ⚡ THE JOLT LOCATOR 🧭");
  Serial.println("   Finding your Monster Energy fix...");
  Serial.println("========================================");
  Serial.println();

  // Initialize GPS on Serial2
  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("[GPS] NEO-6M initialized on Serial2");
  Serial.printf("[GPS] RX=GPIO%d, TX=GPIO%d\n", GPS_RX_PIN, GPS_TX_PIN);

  // Initialize I2C for compass
  Wire.begin(21, 22);  // SDA=GPIO21, SCL=GPIO22
  Serial.println("[I2C] Wire initialized (SDA=21, SCL=22)");

  // Initialize HMC5883L compass
  if (!compass.begin()) {
    Serial.println("[COMPASS] ERROR: HMC5883L not detected!");
    Serial.println("[COMPASS] Check wiring and I2C address.");
    while (1) { delay(100); }
  }
  Serial.println("[COMPASS] HMC5883L initialized successfully");
  
  // Display target coordinates
  Serial.println();
  Serial.println("----------------------------------------");
  Serial.printf("TARGET: Lat %.6f, Lon %.6f\n", TARGET_LAT, TARGET_LON);
  Serial.printf("DECLINATION: %.2f degrees\n", MAGNETIC_DECLINATION);
  Serial.println("----------------------------------------");
  Serial.println();
  Serial.println("Waiting for GPS fix...");
  Serial.println();
}

void loop() {
  // Feed GPS data
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  // Update at specified interval
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();
    updateNavigation();
  }
}

/**
 * Main navigation update function
 */
void updateNavigation() {
  // Check GPS validity
  if (gps.location.isValid() && gps.location.isUpdated()) {
    currentLat = gps.location.lat();
    currentLon = gps.location.lng();
    gpsValid = true;
  }

  // Read compass heading
  float magneticHeading = readCompassHeading();
  float trueHeading = magneticHeading + MAGNETIC_DECLINATION;
  
  // Normalize true heading to 0-360
  trueHeading = normalizeAngle(trueHeading);

  // Print status
  Serial.println("========================================");
  
  if (gpsValid) {
    // Calculate bearing to target
    float targetBearing = calculateBearing(currentLat, currentLon, TARGET_LAT, TARGET_LON);
    
    // Calculate pointer angle (where to point relative to current heading)
    float pointerAngle = targetBearing - trueHeading;
    pointerAngle = normalizeAngle(pointerAngle);
    
    // Convert to -180 to +180 range for easier interpretation
    if (pointerAngle > 180) {
      pointerAngle -= 360;
    }
    
    // Calculate distance
    float distance = calculateDistance(currentLat, currentLon, TARGET_LAT, TARGET_LON);

    Serial.println("📍 CURRENT LOCATION:");
    Serial.printf("   Lat: %.6f\n", currentLat);
    Serial.printf("   Lon: %.6f\n", currentLon);
    Serial.printf("   Satellites: %d\n", gps.satellites.value());
    Serial.println();
    
    Serial.println("🎯 TARGET:");
    Serial.printf("   Lat: %.6f\n", TARGET_LAT);
    Serial.printf("   Lon: %.6f\n", TARGET_LON);
    Serial.printf("   Distance: %.2f km\n", distance);
    Serial.println();
    
    Serial.println("🧭 NAVIGATION:");
    Serial.printf("   Magnetic Heading: %.1f°\n", magneticHeading);
    Serial.printf("   True Heading: %.1f°\n", trueHeading);
    Serial.printf("   Target Bearing: %.1f°\n", targetBearing);
    Serial.println();
    
    Serial.println("⚡ POINTER ANGLE:");
    Serial.printf("   >>> %.1f° <<<\n", pointerAngle);
    printPointerDirection(pointerAngle);
    
  } else {
    Serial.println("⏳ WAITING FOR GPS FIX...");
    Serial.printf("   Satellites in view: %d\n", gps.satellites.value());
    Serial.printf("   Chars processed: %lu\n", gps.charsProcessed());
    Serial.println();
    Serial.println("🧭 COMPASS (preview):");
    Serial.printf("   Magnetic Heading: %.1f°\n", magneticHeading);
    
    if (gps.charsProcessed() < 10) {
      Serial.println();
      Serial.println("⚠️  No GPS data received!");
      Serial.println("   Check GPS wiring (RX/TX may be swapped)");
    }
  }
  
  Serial.println("========================================");
  Serial.println();
}

/**
 * Read heading from HMC5883L magnetometer
 * Returns heading in degrees (0-360)
 */
float readCompassHeading() {
  sensors_event_t event;
  compass.getEvent(&event);
  
  // Calculate heading from X and Y magnetic field components
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // Convert radians to degrees
  heading = heading * 180.0 / PI;
  
  // Normalize to 0-360
  return normalizeAngle(heading);
}

/**
 * Calculate Great Circle Bearing from point 1 to point 2
 * Uses the forward azimuth formula
 * 
 * @param lat1 Latitude of starting point (degrees)
 * @param lon1 Longitude of starting point (degrees)
 * @param lat2 Latitude of destination (degrees)
 * @param lon2 Longitude of destination (degrees)
 * @return Bearing in degrees (0-360)
 */
float calculateBearing(double lat1, double lon1, double lat2, double lon2) {
  // Convert to radians
  double lat1Rad = lat1 * PI / 180.0;
  double lat2Rad = lat2 * PI / 180.0;
  double deltaLon = (lon2 - lon1) * PI / 180.0;
  
  // Great Circle Bearing formula
  double x = sin(deltaLon) * cos(lat2Rad);
  double y = cos(lat1Rad) * sin(lat2Rad) - sin(lat1Rad) * cos(lat2Rad) * cos(deltaLon);
  
  double bearing = atan2(x, y);
  
  // Convert to degrees and normalize
  bearing = bearing * 180.0 / PI;
  return normalizeAngle(bearing);
}

/**
 * Calculate Great Circle Distance between two points
 * Uses the Haversine formula
 * 
 * @return Distance in kilometers
 */
float calculateDistance(double lat1, double lon1, double lat2, double lon2) {
  const double EARTH_RADIUS = 6371.0;  // km
  
  double lat1Rad = lat1 * PI / 180.0;
  double lat2Rad = lat2 * PI / 180.0;
  double deltaLat = (lat2 - lat1) * PI / 180.0;
  double deltaLon = (lon2 - lon1) * PI / 180.0;
  
  double a = sin(deltaLat / 2) * sin(deltaLat / 2) +
             cos(lat1Rad) * cos(lat2Rad) *
             sin(deltaLon / 2) * sin(deltaLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  
  return EARTH_RADIUS * c;
}

/**
 * Normalize angle to 0-360 range
 */
float normalizeAngle(float angle) {
  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;
  return angle;
}

/**
 * Print a visual direction indicator
 */
void printPointerDirection(float angle) {
  String direction;
  
  if (angle >= -22.5 && angle < 22.5) {
    direction = "↑ STRAIGHT AHEAD";
  } else if (angle >= 22.5 && angle < 67.5) {
    direction = "↗ SLIGHT RIGHT";
  } else if (angle >= 67.5 && angle < 112.5) {
    direction = "→ TURN RIGHT";
  } else if (angle >= 112.5 && angle < 157.5) {
    direction = "↘ SHARP RIGHT";
  } else if (angle >= 157.5 || angle < -157.5) {
    direction = "↓ BEHIND YOU";
  } else if (angle >= -157.5 && angle < -112.5) {
    direction = "↙ SHARP LEFT";
  } else if (angle >= -112.5 && angle < -67.5) {
    direction = "← TURN LEFT";
  } else {
    direction = "↖ SLIGHT LEFT";
  }
  
  Serial.printf("   %s\n", direction.c_str());
}
