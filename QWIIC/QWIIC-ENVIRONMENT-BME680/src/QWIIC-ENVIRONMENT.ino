/*
 * Project QWIIC-ENVIRONMENT
 * Description:
 * Author:
 * Date:
 */

/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(SEMI_AUTOMATIC);

#include <Zanshin_BME680.h>  // Include the BME680 Sensor library
/**************************************************************************************************
** Declare all program constants                                                                 **
**************************************************************************************************/
const uint32_t SERIAL_SPEED{115200};  ///< Set the baud rate for Serial I/O

/**************************************************************************************************
** Declare global variables and instantiate classes                                              **
**************************************************************************************************/
BME680_Class BME680;  ///< Create an instance of the BME680 class
///< Forward function declaration with default value for sea level
float altitude(const int32_t press, const float seaLevel = 1013.25);
float altitude(const int32_t press, const float seaLevel) {
  /*!
  @brief     This converts a pressure measurement into a height in meters
  @details   The corrected sea-level pressure can be passed into the function if it is known,
             otherwise the standard atmospheric pressure of 1013.25hPa is used (see
             https://en.wikipedia.org/wiki/Atmospheric_pressure) for details.
  @param[in] press    Pressure reading from BME680
  @param[in] seaLevel Sea-Level pressure in millibars
  @return    floating point altitude in meters.
  */
  static float Altitude;
  Altitude =
      44330.0 * (1.0 - pow(((float)press / 100.0) / seaLevel, 0.1903));  // Convert into meters
  return (Altitude);
}  // of method altitude()

// setup() runs once, when the device is first turned on.
void setup() {
  /*!
  @brief    Arduino method called once at startup to initialize the system
  @details  This is an Arduino IDE method which is called first upon boot or restart. It is only
            called one time and then control goes to the main "loop()" method, from which
            control never returns
  @return   void
  */

  Serial.begin(SERIAL_SPEED);
  waitFor(Serial.isConnected, 5000);

  Serial.print("Starting I2CDemo example program for BME680\n");
  Serial.print("- Initializing BME680 sensor\n");
  while (!BME680.begin(I2C_STANDARD_MODE)) {  // Start BME680 using I2C, use first device found
    Serial.print("-  Unable to find BME680. Trying again in 5 seconds.\n");
    delay(5000);
  }  // of loop until device is located
  Serial.print("- Setting 16x oversampling for all sensors\n");
  BME680.setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
  BME680.setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
  BME680.setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
  Serial.print("- Setting IIR filter to a value of 4 samples\n");
  BME680.setIIRFilter(IIR4);  // Use enumerated type values
  Serial.print("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n");  // "�C" symbols
  BME680.setGas(320, 150);  // 320�c for 150 milliseconds
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  /*!
  @brief    Arduino method for the main program loop
  @details  This is the main program for the Arduino IDE, it is an infinite loop and keeps on
            repeating. The "sprintf()" function is to pretty-print the values, since floating
            point is not supported on the Arduino, split the values into those before and those
            after the decimal point.
  @return   void
  */
  static int32_t  temp, humidity, pressure, gas;  // BME readings
  static char     buf[16];                        // sprintf text buffer
  static float    alt;                            // Temporary variable
  static uint16_t loopCounter = 0;                // Display iterations
  if (loopCounter % 25 == 0) {                    // Show header @25 loops
    Serial.print("\nLoop Temp\xC2\xB0\x43 Humid% Press hPa   Alt m Air m");
    Serial.print("\xE2\x84\xA6\n==== ====== ====== ========= ======= ======\n");  // "�C" symbol
  }                                                     // if-then time to show headers
  BME680.getSensorData(temp, humidity, pressure, gas);  // Get readings
  if (loopCounter++ != 0) {                             // Ignore first reading, might be incorrect
    sprintf(buf, "%4d %3d.%02d", (loopCounter - 1) % 9999,  // Clamp to 9999,
            (int8_t)(temp / 100), (uint8_t)(temp % 100));   // Temp in decidegrees
    Serial.print(buf);
    sprintf(buf, "%3d.%03d", (int8_t)(humidity / 1000),
            (uint16_t)(humidity % 1000));  // Humidity milli-pct
    Serial.print(buf);
    sprintf(buf, "%7d.%02d", (int16_t)(pressure / 100),
            (uint8_t)(pressure % 100));  // Pressure Pascals
    Serial.print(buf);
    alt = altitude(pressure);                                                // temp altitude
    sprintf(buf, "%5d.%02d", (int16_t)(alt), ((uint8_t)(alt * 100) % 100));  // Altitude meters
    Serial.print(buf);
    sprintf(buf, "%4d.%02d\n", (int16_t)(gas / 100), (uint8_t)(gas % 100));  // Resistance milliohms
    Serial.print(buf);
    delay(10000);  // Wait 10s
  }                // of ignore first reading
}