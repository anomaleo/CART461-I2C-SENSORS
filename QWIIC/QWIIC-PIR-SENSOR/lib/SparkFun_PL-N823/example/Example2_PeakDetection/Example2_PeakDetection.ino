/******************************************************************************
  Crude peak detection algorithm. The PL-N823 IR sensor's source voltage peaks
  when something emitting IR crosses it's path. Try adjusting the peak variable
  for your specific application.

  Priyanka Makin @ SparkX Labs
  Original Creation Date: Oct 22, 2019

  This code is Lemonadeware; if you see me (or any other SparkFun employee) at the
  local, and you've found our code helpful, please buy us a round!

  https://www.sparkfun.com/products/15804
  
  Hardware Connections:
  Plug Qwiic IR breakout into Qwiic RedBoard using Qwiic cable.
  Set serial monitor to 9600 baud.

  Distributed as-is; no warranty is given.
******************************************************************************/

#include <SparkFun_ADS1015_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_ADS1015_Arduino_Library
#include <Wire.h>

ADS1015 irsensor;
int dataPrevious = 0;
float multiplier;
int peak = 50;  //Adjust this value as needed

void setup() {
  Serial.begin(9600);
  Wire.begin();   //Join I2C bus

  if (irsensor.begin() == false) {
    Serial.println("Device not found. Check wiring, then restart.");
    while (1);
  }

  multiplier = irsensor.getMultiplier();
}

void loop() {
  int data;
  data = irsensor.getAnalogData(0); //Retrieve raw data value from sensor
  Serial.print("ADC input voltage: ");
  Serial.print(data * multiplier);
  Serial.println("mV");

  //Check if we've seen a peak between two points of data
  if (data * multiplier - dataPrevious * multiplier >= peak && dataPrevious != 0) {
    Serial.println("There's something in front of me!?");
  }
  dataPrevious = data;
  delay(300);
}
