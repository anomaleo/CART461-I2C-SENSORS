
/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC);

#include <SparkFun_LIS2DH12.h> //Click here to get the library: http://librarymanager/All#SparkFun_LIS2DH12
SPARKFUN_LIS2DH12 accel;       //Create instance

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);

  Serial.println("SparkFun Accel Example");

  Wire.begin();

  if (accel.begin() == false)
  {
    Serial.println("Accelerometer not detected. Check address jumper and wiring. Freezing...");
    while (1)
      ;
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  //Print accel values only if new data is available
  if (accel.available())
  {
    float accelX = accel.getX();
    float accelY = accel.getY();
    float accelZ = accel.getZ();
    float tempC = accel.getTemperature();

    Serial.print("Acc [mg]: ");
    Serial.print(accelX, 1);
    Serial.print(" x, ");
    Serial.print(accelY, 1);
    Serial.print(" y, ");
    Serial.print(accelZ, 1);
    Serial.print(" z, ");
    Serial.print(tempC, 1);
    Serial.print("C");
    Serial.println();

    //    int rawX = accel.getRawX();
    //    int rawY = accel.getRawY();
    //    int rawZ = accel.getRawZ();
    //
    //    Serial.print("Acc raw: ");
    //    Serial.print(rawX);
    //    Serial.print(" x, ");
    //    Serial.print(rawY);
    //    Serial.print(" y, ");
    //    Serial.print(rawZ);
    //    Serial.print(" z");
    //    Serial.println();
  }
}