// How to use it (quick)

// Upload this code.

// Open PlatformIO Monitor at 115200.

// Rotate encoder:

// clockwise → count should go up (or down depending on wiring; that’s okay)

// Sanity check: 1 full rotation → angle should be close to 360°

// Type z in the monitor and press Enter → it should zero.

// If your 1 revolution gives something like 90° or 720°, tell me what it prints after one full turn — I’ll tell you the exact COUNTS_PER_REV to set.


#include <Arduino.h>
#include <Encoder.h>

// ---------------- USER SETTINGS ----------------
#define COUNTS_PER_REV 1440.0   // CHANGE THIS if your encoder is different
#define PRINT_PERIOD_MS 100     // print every 100ms
// ------------------------------------------------

#define DEG_PER_COUNT (360.0 / COUNTS_PER_REV)
#define RAD_PER_COUNT (2.0 * PI / COUNTS_PER_REV)

// Encoder pins: CH A -> D2, CH B -> D3
Encoder myEnc(2, 3);

// State
long prevCount = 0;
unsigned long prevTimeMs = 0;
unsigned long lastPrintMs = 0;

float angleDeg = 0.0f;
float angleRad = 0.0f;
float deltaAngleDeg = 0.0f;
float deltaAngleRad = 0.0f;
float omegaRadPerSec = 0.0f;

void zeroEncoder() {
  myEnc.write(0);
  prevCount = 0;

  // reset timing so omega doesn't spike after zero
  prevTimeMs = millis();

  Serial.println("=== ENCODER ZEROED (count=0, angle=0) ===");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Encoder Angle + Delta + Velocity");
  Serial.println("Wiring: VCC->5V, GND->GND, CHA->D2, CHB->D3");
  Serial.println("Commands: send 'z' in Serial Monitor to zero encoder");
  Serial.println("Sanity check: rotate ~1 full rev -> angle should be ~360 deg (if COUNTS_PER_REV is correct)");
  Serial.println("----------------------------------------------------");

  // Step 5: zero at startup
  zeroEncoder();

  prevTimeMs = millis();
  lastPrintMs = millis();
}

void loop() {
  // Optional: zero via serial command
  if (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == 'z' || c == 'Z') {
      zeroEncoder();
    }
  }

  // Read encoder
  long currentCount = myEnc.read();
  long deltaCount = currentCount - prevCount;

  // Compute angles
  angleDeg = currentCount * DEG_PER_COUNT;
  angleRad = currentCount * RAD_PER_COUNT;

  deltaAngleDeg = deltaCount * DEG_PER_COUNT;
  deltaAngleRad = deltaCount * RAD_PER_COUNT;

  // Step 4: compute angular velocity (rad/s) using dt
  unsigned long nowMs = millis();
  float dt = (nowMs - prevTimeMs) / 1000.0f; // seconds

  if (dt > 0.0f) {
    omegaRadPerSec = deltaAngleRad / dt;
  }

  // Update previous values for next loop
  prevCount = currentCount;
  prevTimeMs = nowMs;

  // Step 6: Sanity-check friendly printing at a fixed period
  if (nowMs - lastPrintMs >= PRINT_PERIOD_MS) {
    lastPrintMs = nowMs;

    Serial.print("Count: ");
    Serial.print(currentCount);

    Serial.print(" | Angle(deg): ");
    Serial.print(angleDeg, 2);

    Serial.print(" | dAngle(deg): ");
    Serial.print(deltaAngleDeg, 2);

    Serial.print(" | omega(rad/s): ");
    Serial.print(omegaRadPerSec, 3);

    // Extra sanity hints:
    // direction: if deltaCount > 0 => one direction, deltaCount < 0 => opposite
    Serial.print(" | dir: ");
    if (deltaCount > 0) Serial.print("++");
    else if (deltaCount < 0) Serial.print("--");
    else Serial.print("0");

    Serial.println();
  }
}
