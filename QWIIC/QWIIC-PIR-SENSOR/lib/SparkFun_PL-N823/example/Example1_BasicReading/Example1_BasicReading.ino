/*********************************************************************************
  This example reads the amplified output voltage of the PL-N823 at ADC pin. 
  Will need the SparkFun ADS Library for this example to work.

  Brandon Williams in collaboration with SparkX Labs
  Original Creation Date: September 18th, 2019

  This code is Lemonadeware; if you see me (or any other SparkFun employee) at the 
  local, and you've found our code helpful, please buy us a round!

  https://www.sparkfun.com/products/15804

  Hardware connections:
  Plug Qwiic IR Breakout to RedBoard using Qwiic cable.
  Set Serial monitor to 9600.

  Distributed as-is; no warranty is given.
 *********************************************************************************/

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
