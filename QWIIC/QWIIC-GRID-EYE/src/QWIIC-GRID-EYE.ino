/*
 * Project QWIIC-GRID-EYE
 * Description:
 * Author:
 * Date:
 */

/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(SEMI_AUTOMATIC);

#include <SparkFun_GridEYE_Arduino_Library.h>
GridEYE grideye;

// Use these values (in degrees C) to adjust the contrast
#define HOT 41
#define COLD 13

// This table can be of type int because we map the pixel 
// temperature to 0-3. Temperatures are reported by the 
// library as floats
int pixelTable[64];

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);
 // Start your preferred I2C object 
  Wire.begin();
  // Library assumes "Wire" for I2C but you can pass something else with begin() if you like
  grideye.begin();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {

  // variables to store temperature values
  float testPixelValue = 0;
  float hotPixelValue = 0;
  int hotPixelIndex = 0;

  // for each of the 64 pixels, record the temperature and compare it to the 
  // hottest pixel that we've tested. If it's hotter, that becomes the new
  // king of the hill and its index is recorded. At the end of the loop, we 
  // should have the index and temperature of the hottest pixel in the frame
  for(unsigned char i = 0; i < 64; i++){
    testPixelValue = grideye.getPixelTemperature(i);
      if(testPixelValue > hotPixelValue){
        hotPixelValue = testPixelValue;
        hotPixelIndex = i;
      }
  }

  // Print the result in human readable format
  Serial.print("The hottest pixel is #");
  Serial.print(hotPixelIndex);
  Serial.print(" at ");
  Serial.print(hotPixelValue);
  Serial.println("C");

  // toss in a delay because we don't need to run all out
  delay(500);

}