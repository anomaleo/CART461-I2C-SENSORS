/*
 * Project QWIIC-MICROPRESSURE
 * Description:
 * Author:
 * Date:
 */
/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(SEMI_AUTOMATIC);

#include <SparkFun_MicroPressure.h>
SparkFun_MicroPressure mpr; // Use default values with reset and EOC pins unused

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);

  Wire.begin();

  /* The micropressure sensor uses default settings with the address 0x18 using Wire.

     The mircropressure sensor has a fixed I2C address, if another address is used it
     can be defined here. If you need to use two micropressure sensors, and your
     microcontroller has multiple I2C buses, these parameters can be changed here.

     E.g. mpr.begin(ADDRESS, Wire1)

     Will return true on success or false on failure to communicate. */
  if(!mpr.begin())
  {
    Serial.println("Cannot connect to MicroPressure sensor.");
    while(1);
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
 /* The micropressure sensor outputs pressure readings in pounds per square inch (PSI).
     Optionally, if you prefer pressure in another unit, the library can convert the
     pressure reading to: pascals, kilopascals, bar, torr, inches of murcury, and
     atmospheres.
   */
  Serial.print(mpr.readPressure(),4);
  Serial.println(" PSI");
  Serial.print(mpr.readPressure(PA),1);
  Serial.println(" Pa");
  Serial.print(mpr.readPressure(KPA),4);
  Serial.println(" kPa");
  Serial.print(mpr.readPressure(TORR),3);
  Serial.println(" torr");
  Serial.print(mpr.readPressure(INHG),4);
  Serial.println(" inHg");
  Serial.print(mpr.readPressure(ATM),6);
  Serial.println(" atm");
  Serial.print(mpr.readPressure(BAR),6);
  Serial.println(" bar");
  Serial.println();
  delay(500);
}