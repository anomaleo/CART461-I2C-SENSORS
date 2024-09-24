/********************************************************************************
 * Project PUBLISH AND SUBSCRIBE 
 * Description: This example will illustrate how to fully exploit the Particle 
 * Cloud: PUBLISH, SUBSCRIBE, FUNCTIONS and VARIABLES. Additionally, how to 
 * appropriately handle subscription data received via a Particle.subscription()
 * handler. In order to illustrate the approach, this example program will make
 * user of a LDR (Photocell), 9DoF (IMU - AHRS) and a RGB LED.
 * Author: slab
 * Date: 18022021
 ********************************************************************************/

#include <Particle.h>
#include <Wire.h>

/* IMU IMPLEMENTATION */
#include "Razor.h"
IMU imu;

/* ALWAYS RUN PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */ 
SYSTEM_THREAD(ENABLED);

/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC); // DONE BY DEFAULT
//SYSTEM_MODE(SEMI_AUTOMATIC); // CONTROL WHEN & HOW TO CONNECT TO PARTICLE CLOUD (WIFI RADIO ON)
//SYSTEM_MODE(MANUAL); // CONTROL WHEN & HOW TO CONNECT TO WIFI NETWORK

#define DEBUG_LED D7 // SMALL BLUE LED NEXT USB CONNECTOR (RIGHT OF USB)
#define R_LED     D6 // RED LED
#define G_LED     D5 // GREEN LED
#define B_LED     D4 // BLUE LED
#define LDR       A5 // ANALOG_IN = ( 0 ... 4095 ) => 2^12 bits.
#define B_TN      D2 // MOMENTARY BUTTON

/* TIMER */
unsigned long timed = 0;
unsigned long timepassed;

/* VARIABLES USED FOR BOTH PARTICLE CLOUD VARIABLES AND PARTICLE SUBSCRIPTION
  - IMU ACCELERO, MAGNETO, GYRO */
float * accel;  
float * magnetom;
float * gyro;
float yaw, pitch, roll, magnitude;

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

int rgb(const char* arg) {
  if (strcmp(arg, "red") == 0) { digitalWrite(R_LED, HIGH); Serial.println("red on"); }
  if (strcmp(arg, "green") == 0) { digitalWrite(G_LED, HIGH); Serial.println("green on"); }
  if (strcmp(arg, "blue") == 0) { digitalWrite(B_LED, HIGH); Serial.println("blue on"); }
  if (strcmp(arg, "off") == 0) { digitalWrite(R_LED, LOW); digitalWrite(G_LED, LOW); digitalWrite(B_LED, LOW); Serial.println("all off");}
  return 0;
}

void setup() {

  /* PARTICLE CLOUD INTENTIONS ALWAYS DECLARED AND INIT'ED IN VOID SETUP */
  Particle.variable("location", "ME @ MTL", STRING);
  Particle.variable("accelerometer", accelerometer);
  Particle.variable("magnetometer", magnetometer);
  Particle.variable("gyrometer", gyrometer);
  Particle.variable("yawpitchrollmagnitude", yawPitchRollMagnitude);
  Particle.function("rgb", rgb);

  /* PARTICLE PUBLISH 'n' SUBSCRIBE ARE MULTICAST OPERATIONS - FULLY CONNECTED 
    SIMPLY SUBSCRIBE TO A KNOWN DATA SOURCE OR PUBLISH DATA AS SOURCE
    THE ONLY CAVEAT - ONE (1) MSG PER SECOND OVER THE CLOUD 
  */

  /* SUBSCRIBE TO SIMPLE STATE CHANGE OF RGB LED */
  Particle.subscribe("cslab-proline-rgb", cslabprolinergb);

  /* SUBSCRIBE TO AGGREGATED STATE OF IMU */
  Particle.subscribe("cslab-proline-imu", cslabprolineimu);

  Serial.begin(57600);
  //while( !Serial.isConnected() ) // wait for Host to open serial port
  waitFor(Serial.isConnected, 5000); 
  timepassed = timed;

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

  timed = millis();
  if (timed - timepassed > 1002 ) {
    /* UPDATE IMU READINGS: ACCELERO[3], MAGENTO[3], GYRO[3], YAW, PITCH, ROLL, HEADING */
    /* ACCESS UPDATED IMU STATE */
    accel = imu.getAccelerometer();  
    magnetom = imu.getMagnetometer();
    gyro = imu.getGyrometer();
    yaw = imu.getYaw();
    pitch = imu.getPitch();
    roll = imu.getRoll();
    magnitude = imu.getMagnitude();

    /* 
      PREPARE DATA TO BE PUBLISHED ONTO PARTICLE CLOUD - USE snprintf() to create formatted string 
      STRING FORMAT (COMMA DELIMITED):
      accel[0],accel[1],accel[2],magnetom[0],magnetom[1],magnetom[2],gyro[0],gyro[1],gyro[2],yaw, pitch, roll, heading  
    */
    snprintf(buffer, sizeof(buffer), "%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f", \
    accel[0],accel[1],accel[2],magnetom[0],magnetom[1],magnetom[2],gyro[0],gyro[1],gyro[2],yaw,pitch,roll,magnitude);

    // if( Particle.publish("cslab-proline-imu", buffer) ) Serial.println ("cslab-proline-imu published");
    /* PARTICLE CLOUD PUBLISH DELAY 1000ms */
    delay(1002);
    timepassed = millis();
  }

  if( digitalRead(B_TN) == LOW ) {
    static int _cnt = 0;
    /* EVENTS CAN ONLY BE PUBLISHED ONCE PER SECOND */
    digitalWrite(DEBUG_LED, HIGH); 
    delay(50);
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, LOW);

    switch(_cnt % 4) {
      case 0: _stateofrgbled = "red"; _cnt++; digitalWrite(R_LED, HIGH); break;
      case 1: _stateofrgbled = "green"; _cnt++; digitalWrite(G_LED, HIGH);break;
      case 2: _stateofrgbled = "blue"; _cnt++; digitalWrite(B_LED, HIGH);break;
      case 3: _stateofrgbled = "off"; _cnt++; break;
    }
    if(_cnt >= 255) _cnt = 0;
    //Particle.publish("a-ble-centralBTN", "GUDRUN", PRIVATE); //ONLY PRIVATE OR DEVICE ID
    //bool success = Particle.publish("a-ble-centralBTN", "1", PRIVATE);
    if( Particle.publish("cslab-proline-rgb", _stateofrgbled ) ) Serial.println ("cslab-proline-rgb published");
    delay(1002);

    digitalWrite(DEBUG_LED, LOW);
  }
}

/* PARTICLE CLOUD SUBSCRIBE CALLBACK */
void cslabprolinergb(const char *event, const char *data) { 

/* 
  Particle.subscribe handlers are void functions. They take two variables the name of your event, 
  and any data (UTF-8) that goes along with your event.
  In our case, the event will be "cslabprolinergb" and the data will be "red", "green", "blue" or "off". 
  We're going to use strcmp(), which compares two chars strings. If they are the same, strcmp will return 0.
  */
  /* SIMPLE VERIFICATION OF *data */
  Serial.println();
  Serial.println("SUBSCRIPTION *data - needs to be tokenised: ");

  if (strcmp(data, "red") == 0) { Serial.println("red led is on"); }
  if (strcmp(data, "green") == 0) { Serial.println("green led is on"); }
  if (strcmp(data, "blue") == 0) { Serial.println("blue led is on"); }
  if (strcmp(data, "off") == 0) { Serial.println("led is off"); }
  Serial.println();
}

#include <vector>
using namespace std;
void cslabprolineimu(const char *event, const char *data) {
  /* cslabprolineimu event expects *data to contain (point-to):
    STRING FORMAT (COMMA DELIMITED):
    accel[0],accel[1],accel[2],magnetom[0],magnetom[1],magnetom[2],gyro[0],gyro[1],gyro[2],yaw, pitch, roll, heading
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
          Serial.print("Accelerometer: ");
          Serial.print("accel[X]: "); Serial.print(result.at(0).c_str()); 
          Serial.print(" accel[Y]: "); Serial.print(result.at(1).c_str()); 
          Serial.print(" accel[Z]: "); Serial.println(result.at(2).c_str()); 
          break;
        case 3:
          Serial.print("Magnetometer: ");
          Serial.print("magno[X]: "); Serial.print(result.at(3).c_str()); 
          Serial.print(" magno[Y]: "); Serial.print(result.at(4).c_str()); 
          Serial.print(" magno[Z]: "); Serial.println(result.at(5).c_str()); 
          break;
        case 6:
          Serial.print("Gyrometer: ");
          Serial.print("gyro[X]: "); Serial.print(result.at(6).c_str()); 
          Serial.print(" gyro[Y]: "); Serial.print(result.at(7).c_str()); 
          Serial.print(" gyro[Z]: "); Serial.println(result.at(8).c_str()); 
          break;
        case 9:
          Serial.print("Yaw, Roll and Pitch: ");
          Serial.print("yaw: "); Serial.print(result.at(9).c_str()); 
          Serial.print(" roll: "); Serial.print(result.at(10).c_str()); 
          Serial.print(" pitch: "); Serial.println(result.at(11).c_str()); 
          break;
        case 12:
          Serial.print("Magnitude : ");
          Serial.print("mag: "); Serial.println(result.at(12).c_str()); 
          break;
        }
    }
    Serial.println();
}