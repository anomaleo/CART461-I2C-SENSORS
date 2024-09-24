/*
 * Project QWIIC-RGB
 * Description:
 * Author:
 * Date:
 */
/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC);

#include <SparkFun_BH1749NUC_Arduino_Library.h>
BH1749NUC rgb;

#include <SparkFun_PCA9536_Arduino_Library.h>
PCA9536 io;

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);

  if (rgb.begin() != BH1749NUC_SUCCESS) {
    Serial.println("Error initializing the rgb sensor.");
    while (1);
  }
  
  // IR and RGB gain can be set to either BH1749NUC_GAIN_X1 or
  // BH1749NUC_GAIN_X32
  rgb.setIRGain(BH1749NUC_GAIN_X1);
  rgb.setRGBGain(BH1749NUC_GAIN_X1);
  // Measurement mode can be set to either BH1749NUC_MEASUREMENT_MODE_35_MS,
  // BH1749NUC_MEASUREMENT_MODE_120_MS, or BH1749NUC_MEASUREMENT_MODE_240_MS
  // (35ms, 120ms, 240ms between measurements).
  rgb.setMeasurementMode(BH1749NUC_MEASUREMENT_MODE_240_MS);

  // Initialize the PCA9536 with a begin function
  if (io.begin() == false) {
    Serial.println("PCA9536 not detected. Please check wiring. Freezing...");
    while (1);
  }

  for (int i = 0; i < 4; i++){
    // pinMode can be used to set an I/O as OUTPUT or INPUT
    io.pinMode(i, OUTPUT);
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  if (rgb.available()) {
    Serial.println("RGB SENSED VALUES");
    Serial.println("Red: " + String(rgb.colors.red));
    Serial.println("Green: " + String(rgb.colors.green));
    Serial.println("Blue: " + String(rgb.colors.blue));
    Serial.println("IR: " + String(rgb.colors.ir));
    Serial.println("Green2: " + String(rgb.colors.green2));
    Serial.println();
  }
  delay(1000);

  // CONTROL SUPPORT LED WAVE COLOUR
  for (int i = 3; i >= 0; i--) {
    // digitalWrite or write can be used to set an I/O value
    // (both perform the same function)
    io.digitalWrite((i + 1) % 4, HIGH); // Turn last pin HIGH
    io.write(i, LOW);                   // Turn this pin LOW
    delay(1000);
  }
}

