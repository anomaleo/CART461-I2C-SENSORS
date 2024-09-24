/*
 * Project QWIIC-TRIAD-SPECTROSCOPY
 * Description:
 * Author:
 * Date:
 */

/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC);

#include <SparkFun_AS7265X.h> //Click here to get the library: http://librarymanager/All#SparkFun_AS7265X
AS7265X sensor;


// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);

  Serial.println("AS7265x Spectral Triad Example");

  Serial.println("Point the Triad away and press a key to begin with illumination...");
  
  Serial.read(); //Throw away the user's button

  if(sensor.begin() == false) {
    Serial.println("Sensor does not appear to be connected. Please check wiring. Freezing...");
    while(1);
  }

  sensor.disableIndicator(); //Turn off the blue status LED

  Serial.println("A,B,C,D,E,F,G,H,I,J,K,L,R,S,T,U,V,W");
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  sensor.takeMeasurementsWithBulb(); //This is a hard wait while all 18 channels are measured

  Serial.print(sensor.getCalibratedA());
  Serial.print(",");
  Serial.print(sensor.getCalibratedB());
  Serial.print(",");
  Serial.print(sensor.getCalibratedC());
  Serial.print(",");
  Serial.print(sensor.getCalibratedD());
  Serial.print(",");
  Serial.print(sensor.getCalibratedE());
  Serial.print(",");
  Serial.print(sensor.getCalibratedF());
  Serial.print(",");

  Serial.print(sensor.getCalibratedG());
  Serial.print(",");
  Serial.print(sensor.getCalibratedH());
  Serial.print(",");
  Serial.print(sensor.getCalibratedI());
  Serial.print(",");
  Serial.print(sensor.getCalibratedJ());
  Serial.print(",");
  Serial.print(sensor.getCalibratedK());
  Serial.print(",");
  Serial.print(sensor.getCalibratedL());
  Serial.print(",");

  Serial.print(sensor.getCalibratedR());
  Serial.print(",");
  Serial.print(sensor.getCalibratedS());
  Serial.print(",");
  Serial.print(sensor.getCalibratedT());
  Serial.print(",");
  Serial.print(sensor.getCalibratedU());
  Serial.print(",");
  Serial.print(sensor.getCalibratedV());
  Serial.print(",");
  Serial.print(sensor.getCalibratedW());
  Serial.print(",");

  Serial.println();
}