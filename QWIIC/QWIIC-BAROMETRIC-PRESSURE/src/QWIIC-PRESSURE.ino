/*
 * Project QWIIC-PRESSURE
 * Description:
 * Author:
 * Date:
 */

/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(SEMI_AUTOMATIC);

#include <SparkFun_LPS25HB_Arduino_Library.h>  // Click here to get the library: http://librarymanager/All#SparkFun_LPS25HB
LPS25HB pressureSensor; // Create an object of the LPS25HB class

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);

  Serial.println("LPS25HB Pressure Sensor Example 5 - FIFO Averaging");
  Serial.println();

  Wire.begin();
  Wire.setClock(400000); 
  pressureSensor.begin();                                      // Begin links an I2C port and I2C address to the sensor, and begins I2C on the main board

  while(pressureSensor.isConnected() == false)
  {
    Serial.print("Waiting for connection to LPS25HB.");                         // Alert the user that the device cannot be reached
    Serial.print(" Are you using the right Wire port and I2C address?");        // Suggest possible fixes
    Serial.println(" See Example2_I2C_Configuration for how to change these."); // Example2 illustrates how to change the Wire port or I2C address
    delay(250);
  }

  pressureSensor.setFIFOMeanNum(LPS25HB_FIFO_CTRL_M_32);       // Specifies the desired number of moving average samples. Valid values are 2, 4, 8, 16, and 32
  pressureSensor.setFIFOMode(LPS25HB_FIFO_CTRL_MEAN);          // Sets the FIFO to the MEAN mode, which implements a hardware moving average

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  if(pressureSensor.isConnected() == true)
  {
    if(pressureSensor.getStatus() == 0x00){pressureSensor.begin();}                                 // If it is connected but not responding (for example after a hot-swap) then it may need to be re-initialized
    Serial.print("Connected. Sensor Status: "); 
    Serial.print(pressureSensor.getStatus(),HEX);        // Read the sensor status, the datasheet can explain what the various codes mean
    Serial.print("Pressure in hPa: "); 
    Serial.print(pressureSensor.getPressure_hPa());               // Get the pressure reading in hPa as determined by dividing the number of ADC counts by 4096 (according to the datasheet)
    Serial.print(", Temperature (degC): "); 
    Serial.println(pressureSensor.getTemperature_degC());    // Get the temperature in degrees C by dividing the ADC count by 480
  }
  else
  {
    Serial.println("Disconnected");
    pressureSensor.begin();
  }
  delay(100);
}