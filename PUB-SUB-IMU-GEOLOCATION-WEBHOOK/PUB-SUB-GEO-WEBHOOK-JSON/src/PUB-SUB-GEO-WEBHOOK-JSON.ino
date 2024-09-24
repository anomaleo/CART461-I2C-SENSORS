/********************************************************************************
 * Project PUB-SUB GEOLOACTION 
 * Description: This example will illustrate how to fully exploit the Particle 
 * Cloud: PUBLISH, SUBSCRIBE, FUNCTIONS and VARIABLES and utilise Particle 
 * Google GEOLOCATION INTEGRATION (LONGITUDE, LATITUDE, ACCURACY). We will
 * Publish to Particle Cloud and utilise Particle Cloud Webhook to transmit data to 
 * a remote server outside Particle Cloud Platform.  
 *  
 * 
 * We will continue handle subscription data received via a Particle.subscription() 
 * handler. In order to illustrate the approach, this example program will make of 
 * a 9DoF (IMU - AHRS).
 * Author: slab
 * Date: 18022021
 ********************************************************************************/

#include <Particle.h>
#include <Wire.h>

/* IMU IMPLEMENTATION */
#include "Razor.h"
IMU imu;

/* PARTICLE CLOUD GOOGLE MAPS INTEGRATION */
#include "google-maps-device-locator.h"
GoogleMapsDeviceLocator locator;

/* JSON PARSER AND GENERATOR */
#include "JsonParserGeneratorRK.h"

/* ALWAYS RUN PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */ 
SYSTEM_THREAD(ENABLED);

/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC); // DONE BY DEFAULT

#define DEBUG_LED D7 // SMALL BLUE LED NEXT USB CONNECTOR (RIGHT OF USB)
#define R_LED     D6 // RED LED
#define G_LED     D5 // GREEN LED
#define B_LED     D4 // BLUE LED
#define LDR       A5 // ANALOG_IN = ( 0 ... 4095 ) => 2^12 bits.
#define B_TN      D2 // MOMENTARY BUTTON

/* VARIABLES USED FOR BOTH PARTICLE CLOUD VARIABLES AND PARTICLE SUBSCRIPTION
  - IMU ACCELERO, MAGNETO, GYRO */
float * accel;  
float * magnetom;
float * gyro;
float yaw, pitch, roll, magnitude, latitude, longitude, accuracy;

char buffer[256];
char * _stateofrgbled;

/* PARTICLE CLOUD API VARIABLES */
String accelerometer() {
  snprintf(buffer, sizeof(buffer), "%.02f,%.02f,%.02f", accel[0], accel[1], accel[2]);
  return buffer;
}

String magnetometer() {
  snprintf(buffer, sizeof(buffer), "%.02f,%.02f,%.02f", magnetom[0],magnetom[1],magnetom[2]);
  return buffer;
}

String gyrometer() {
  snprintf(buffer, sizeof(buffer), "%.02f,%.02f,%.02f", gyro[0],gyro[1],gyro[2]);
  return buffer;
}

String yawPitchRollMagnitude() {
  snprintf(buffer, sizeof(buffer), "%.02f,%.02f,%.02f,%.02f", yaw, pitch, roll, magnitude);
  return buffer;
}

String geolocation() {
  snprintf(buffer, sizeof(buffer), "%.07f,%.07f,%.07f", latitude, longitude, accuracy);
  return buffer;
}

void setup() {
  /* PARTICLE CLOUD INTENTIONS ALWAYS DECLARED AND INIT'ED IN VOID SETUP */
  Particle.variable("location", "ME @ MTL", STRING);
  Particle.variable("accelerometer", accelerometer);
  Particle.variable("magnetometer", magnetometer);
  Particle.variable("gyrometer", gyrometer);
  Particle.variable("yawpitchrollmagnitude", yawPitchRollMagnitude);
  Particle.variable("geolocation", geolocation);

  /* SUBSCRIBE TO AGGREGATED STATE OF IMU */
  Particle.subscribe("cslab-asparagine-imu-geo", cslabasparagineimugeo);

  /* GEOLATION: LAT, LONG AND ACCRUACY - GOOGLE MAPS */
  locator.withEventName("cslablocation");
  locator.withSubscribe(locationCallback).withLocateOnce();
  //locator.withLocatePeriodic(32) // GEOLOCATE EVERY 32 SECONDS
  //locator.withSubscribe(locationCallback); // REQUIRES AN EVENT

  Serial.begin(57600);
  //while( !Serial.isConnected() ) // wait for Host to open serial port
  waitFor(Serial.isConnected, 5000); 

  /* BLUE DEBUG LED */
  pinMode(DEBUG_LED, OUTPUT);

  /* RGB LED */
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);

  /* INPUT: PUSH BUTTON */
  pinMode(B_TN, INPUT_PULLUP); // NO RESISTOR

  delay(5); // Force Serial.println in void setup()
  Serial.println("Completed void setup");
}

void loop() { 
  /* IMU RUN @ 50Hz */
  imu.loop();

  /* UPDATE IMU READINGS: ACCELERO[3], MAGENTO[3], GYRO[3], YAW, PITCH, ROLL, HEADING */
  /* ACCESS UPDATED IMU STATE */
  accel = imu.getAccelerometer();  
  magnetom = imu.getMagnetometer();
  gyro = imu.getGyrometer();
  yaw = imu.getYaw();
  pitch = imu.getPitch();
  roll = imu.getRoll();
  magnitude = imu.getMagnitude();

  /* GOOGLE GEOLOCATION LOOP - OPTIONAL (CAN BE REMOVED) */
  locator.loop(); // 

  /* EVENT WHICH INDUCES A GOOGLE GEOLOCATION REQUEST & PARTICLE PUBLISH ROUTINE */
  if( digitalRead(B_TN) == LOW ) {
    /* REQUEST LOCATION FROM GOOGLE */
    locator.publishLocation();

    digitalWrite(R_LED, HIGH);
    delay(1000);
  }
}

/* PARTICLE CLOUD GOOGLE GEOLATION: LAT, LONG AND ACCRUACY - CALLBACK */
void locationCallback(float lat, float lon, float accu) {
  // Handle the returned location data for the device. This method is passed three arguments:
  // - Latitude
  // - Longitude
  // - Accuracy of estimated location (in meters)
  latitude = lat;
  longitude = lon;
  accuracy = accu;

  digitalWrite(R_LED, LOW);
  digitalWrite(B_LED, HIGH);
  delay(1000);
  
  /* IN THIS PROGRAM - OUR INTENTION IS TO PUBLISH DATA TO THE PARTICLE CLOUD PLATFORM */
  /* PREPARE DATA TO BE PUBLISHED ONTO PARTICLE CLOUD */
  snprintf(buffer, sizeof(buffer), "%.07f,%.07f,%.07f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f", \
  lat, lon, accu, accel[0],accel[1],accel[2],magnetom[0],magnetom[1],magnetom[2],gyro[0],gyro[1],gyro[2],yaw,pitch,roll,magnitude);

  if( Particle.publish("cslab-asparagine-imu-geo", buffer) ) Serial.println ("cslab-asparagine-imu-geo published");

  /* ADDITIONALLY, WE WANT TO PUBLISH DATA TO A SERVER NOT WITHIN THE PARTICLE CLOUD PLATFORM */
  /* HERE WE CAN PREPARE THE DATA TO TRANSMIT VIA A WEBHOOK (HTTPS SERVER) USING A STRING FORMATTED AS JSON OBJECT */
  // snprintf(buffer, sizeof(buffer),"{\"device-name\":\"%s\",\"lat\":%.07f,\"lon\":%.07f,\"accu\":%.07f,\"accel-x\":%.02f,\"accel-y\":%.02f,\
  // \"accel-z\":%.02f,\"magnetom-x\":%.02f,\"magnetom-y\":%.02f,\"magnetom-z\":%.02f,\"gyro-x\":%.02f,\"gyro-y\":%.02f,\
  // \"gyro-z\":%.02f,\"yaw\":%.02f,\"pitch\":%.02f,\"roll\":%.02f,\"magnitude\":%.02f}",\
  // "cslab-proline",lat,lon,accuracy,accel[0],accel[1],accel[2],magnetom[0],magnetom[1],magnetom[2],gyro[0],gyro[1],gyro[2],\
  // yaw,pitch,roll,magnitude );
  /* ^^^ NASTY AND ERROR PRONE ^^^ */

/* ALTERNATIVE TO snprinf(). JsonWriterStatic creates a buffer to hold up to 512 bytes of 
  JSON data (good for Particle.publish) */
  JsonWriterStatic<512> jw;
	{
		JsonWriterAutoObject obj(&jw);
		jw.insertKeyValue("device-name", "cslab-asparagine");
		jw.insertKeyValue("lat", lat);
    jw.insertKeyValue("lon", lon);
    jw.insertKeyValue("accu", accu);
    jw.insertKeyValue("accel-x", accel[0]);
    jw.insertKeyValue("accel-y", accel[1]);
    jw.insertKeyValue("accel-z", accel[2]);
    jw.insertKeyValue("magnetom-x", magnetom[0]);
    jw.insertKeyValue("magnetom-y", magnetom[1]);
    jw.insertKeyValue("magnetom-z", magnetom[2]);
    jw.insertKeyValue("gyro-x", gyro[0]);
    jw.insertKeyValue("gyro-y", gyro[1]);
    jw.insertKeyValue("gyro-z", gyro[3]);
		jw.insertKeyValue("yaw", yaw);
		jw.insertKeyValue("pitch", pitch);
		jw.insertKeyValue("roll", roll);
		jw.insertKeyValue("magnitude", magnitude);
	}

  Serial.println(jw.getBuffer());
    
  /* TRANSMIT DATA TO A HTTPS SERVER OUT OF PARTICLE CLOUD PLATFORM - USE A WEBHOOK */ 
  if ( Particle.publish("cslabasparaginetohttps", jw.getBuffer()) ) Serial.println ("cslab-asparagine-imu-geo published to https server");

  digitalWrite(B_LED, LOW);
  digitalWrite(G_LED, HIGH);
  delay(1002);
  digitalWrite(G_LED, LOW);
}


/* PARTICLE CLOUD SUBSCRIBE CALLBACK */
#include <vector>
using namespace std;
void cslabasparagineimugeo(const char *event, const char *data) {
  /* cslabprolineimu event expects *data to contain (point-to):
    STRING FORMAT (COMMA DELIMITED):
    latitude,longitude,accuracy,accel[0],accel[1],accel[2],magnetom[0],magnetom[1],magnetom[2],gyro[0],gyro[1],gyro[2],yaw, pitch, roll, heading
  */

  /* SIMPLE VERIFICATION OF *data */
  Serial.println();
  Serial.println("SUBSCRIPTION *data as String - needs to be tokenised: ");
  Serial.println(data);
  Serial.println();

  /* EXAMPLE TOKENISER */
  vector<string> result;
  char _tempdata[strlen(data)];
  strcpy(_tempdata, data);

  char* token; 
  // Get the first token followed by the delimiter ','
  token = strtok(_tempdata, ",");

  while (token != NULL) { 
    //add to the result array
    result.push_back(token);
    // strtok() contains a static pointer to the previous passed string
    token = strtok(NULL, ","); 
  } 

    for(int i=0; i < result.size();i += 3){
        //Serial.println( result.at(i).c_str() );
        switch(i) {
        case 0:
          Serial.print("Geolocation: ");
          Serial.print("latitude: "); Serial.print(result.at(0).c_str()); 
          Serial.print(" longitude: "); Serial.print(result.at(1).c_str()); 
          Serial.print(" accuracy: "); Serial.println(result.at(2).c_str()); 
          break;
        case 3:
          Serial.print("Accelerometer: ");
          Serial.print("accel[X]: "); Serial.print(result.at(3).c_str()); 
          Serial.print(" accel[Y]: "); Serial.print(result.at(4).c_str()); 
          Serial.print(" accel[Z]: "); Serial.println(result.at(5).c_str()); 
          break;
        case 6:
          Serial.print("Magnetometer: ");
          Serial.print("magno[X]: "); Serial.print(result.at(6).c_str()); 
          Serial.print(" magno[Y]: "); Serial.print(result.at(7).c_str()); 
          Serial.print(" magno[Z]: "); Serial.println(result.at(8).c_str()); 
          break;
        case 9:
          Serial.print("Gyrometer: ");
          Serial.print("gyro[X]: "); Serial.print(result.at(9).c_str()); 
          Serial.print(" gyro[Y]: "); Serial.print(result.at(10).c_str()); 
          Serial.print(" gyro[Z]: "); Serial.println(result.at(11).c_str()); 
          break;
        case 12:
          Serial.print("Yaw, Roll and Pitch: ");
          Serial.print("yaw: "); Serial.print(result.at(12).c_str()); 
          Serial.print(" roll: "); Serial.print(result.at(13).c_str()); 
          Serial.print(" pitch: "); Serial.println(result.at(14).c_str()); 
          break;
        case 15:
          Serial.print("Magnitude : ");
          Serial.print("mag: "); Serial.println(result.at(15).c_str()); 
          break;
        }
    }
    Serial.println();
}