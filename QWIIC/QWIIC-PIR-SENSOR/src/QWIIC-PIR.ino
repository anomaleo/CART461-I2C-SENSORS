/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC);

#include <SparkFun_ADS1015_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_ADS1015_Arduino_Library
#include <Wire.h>

ADS1015 irsensor;                   //Initialize using ADS1015 library
float multiplier;

void setup() {
  Serial.begin(9600);
  Wire.begin();   //Join I2C bus

  if(irsensor.begin() == false){
    Serial.println("Device not found. Check wiring, then restart.");
    while(1);
  }
  multiplier = irsensor.getMultiplier();
}

void loop() {
  int data;
  data = irsensor.getAnalogData(0);   //Retrieve raw data value from sensor
  Serial.print("ADC input voltage: ");
  Serial.print(data * multiplier);
  Serial.println("mV");
  delay(50);                         //Sample data reading every half second
}