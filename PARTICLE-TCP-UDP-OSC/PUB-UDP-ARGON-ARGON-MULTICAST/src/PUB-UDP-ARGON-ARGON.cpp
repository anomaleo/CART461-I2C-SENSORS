/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/eo/Desktop/CART461/PARTICLE-TCP-UDP-OSC/PUB-UDP-ARGON-ARGON-MULTICAST/src/PUB-UDP-ARGON-ARGON.ino"
/*
 * Project ARGON <-> ARGON VIA UDP (USER DATAGRAMS PROTOCOL)
 * Description: UDP does not guarantee that messages are always delivered, 
 * or that they are delivered in the order supplied. In cases where your 
 * application requires a reliable connection, TCPClient is the alternative.
 * 
 * BUFFERED UDP COMMUNICATION && MULTICAST (LOCAL NETWORK) GROUP.
 * https://en.wikipedia.org/wiki/Multicast
 * Multicast is group communication where data transmission is addressed to a 
 * group of destination computers simultaneously. Multicast can be one-to-many
 * or many-to-many distribution.
 */

#include <Particle.h>
#include <Wire.h>
#include <vector>
void connectToParticleCloud();
void disconnectFromParticleCloud();
void connectToLAN();
void setup();
void loop();
void uuudeepee();
void dof();
void locationCallback(float lat, float lon, float accu);
#line 17 "/Users/eo/Desktop/CART461/PARTICLE-TCP-UDP-OSC/PUB-UDP-ARGON-ARGON-MULTICAST/src/PUB-UDP-ARGON-ARGON.ino"
using namespace std;

/* PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* CONTROL WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC);

/* PARTICLE CLOUD GOOGLE MAPS INTEGRATION */
#include "google-maps-device-locator.h"
GoogleMapsDeviceLocator locator;

/* IMU IMPLEMENTATION */
#include "Razor.h"
IMU imu;

/* UDP AS TRANSPORT LAYER */
UDP udp;
vector<string> inudp;
IPAddress myArgonIP;
//IPAddress multicastAddress(224,0,1,32);
/* EXPLICIT REMOTE ADDRESS DECLARATION IF IS KNOWN - 
 * REMOTE ADDRESS CAN ALSO BE RETRIEVED FROM RECEIVED 
 * udp.remoteIP() in received PACKET  */
static uint8_t argons[6][4] = { 
  { 10, 0, 10, 27 },
  { 10, 0, 1, 6 },
  { 10, 0, 1, 7 },
  { 10, 0, 1, 8 },
  { 224, 0, 1, 32 }, // MULTICAST GROUP #1
  { 224, 0, 1, 16 }  // MULTICAST GROUP #2
};
#define SELFTEST 0
/* PORTS FOR INCOMING & OUTGOING DATA */
#define LISTEN  8001
#define INPORT  8001

/* ONBOARD LED = DEBUG LED */
#define DEBUG_LED D7 // SMALL BLUE LED NEXT USB CONNECTOR (RIGHT OF USB)
#define R_LED     D6 // RED LED
#define G_LED     D5 // GREEN LED
#define B_LED     D4 // BLUE LED
#define B_TN      D2 // MOMENTARY BUTTON

/* VARIABLES USED FOR BOTH PARTICLE CLOUD VARIABLES AND PARTICLE SUBSCRIPTION
  - IMU ACCELERO, MAGNETO, GYRO */
float * accel;
float * magnetom;
float * gyro;
float yaw, pitch, roll, magnitude;

unsigned long int cTime = -1;

/* PARTICLE CLOUD SERVICE - ONLY IF SHARING DATA WITH OTHER ARGONS */
void connectToParticleCloud() {
    if( !Particle.connected() ) {
        Particle.connect();
    }
    waitUntil( Particle.connected );
}

/* PARTICLE CLOUD SERVICE - ONLY IF SHARING DATA WITH OTHER ARGONS */
void disconnectFromParticleCloud() {
    if( Particle.connected() ) {
        Particle.disconnect();
    }
    waitUntil( Particle.disconnected );
}

void connectToLAN() {
  /* IF ARGON ALREADY CONFIGURED FOR SSID/ROUTER - THEN THIS */
  /* TRY CONNECT TO SSID - ROUTER */
  WiFi.connect();
  /* WAIT UNTIL DHCP SERVICE ASSIGNS ARGON IPADDRESS */
  while(!WiFi.ready());

  delay(5);
 /* GET HOST (ARGON) ASSIGNED IP */
  Serial.print("ARGON IP (DHCP): ");
  Serial.println(myArgonIP = WiFi.localIP());
}

void setup() {
  /* NO PARTICLE CLOUD VARIABLE OR FUNCTIONS */
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000); 

  /* GEOLATION: LAT, LONG AND ACCRUACY - GOOGLE MAPS */
  locator.withEventName("cslablocation");
  locator.withSubscribe(locationCallback).withLocateOnce();

  /* BLUE DEBUG LED */
  pinMode(DEBUG_LED, OUTPUT);

  /* RGB LED */
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);

  /* INPUT: PUSH BUTTON */
  pinMode(B_TN, INPUT_PULLUP); // NO RESISTOR

  connectToLAN();
  /* START UDP SERVICE */
  if (WiFi.ready()) {
    /* MULTICAST - WE DO NOT "LISTEN" */
    udp.begin(LISTEN);
    udp.joinMulticast(argons[4]);
  }
  
  delay(5); // Force Serial.println in void setup()
  Serial.println("Completed void setup");
}

void loop() {
  /* CHECK FOR UDP DATA */
  uuudeepee();

  /* IMU RUN @ 50Hz */
  imu.loop();

  /* TRANSMIT UDP PACKET @ 1Hz */
  if( millis() - cTime > 100) {
    dof();
    cTime = millis();
  }

  /* GOOGLE GEOLOCATION LOOP - OPTIONAL (CAN BE REMOVED) */
  locator.loop(); // 

  /* EVENT WHICH INDUCES A GOOGLE GEOLOCATION REQUEST & PARTICLE PUBLISH ROUTINE */
  if( digitalRead(B_TN) == LOW ) {
    /* WEBHOOK REQUEST - CONNECT TO PARTICLE CLOUD */
    connectToParticleCloud();
    /* REQUEST LOCATION FROM GOOGLE */
    locator.publishLocation();
  }
}

/* UDP - BUFFERED UNMARSHALLING ALWAYS FOLLOWS THREE STEPS:
   a) udp.parsePacket() (UDP DATA PACKET LENGTH) > 0
   b) udp.read(_rBuffer, sized) (OPTIMAL) * _or_
   b) udp.available() (READ/ACCESS ENTIRE PACKET)
      while(udp.available()) 
        _rBuffer[cnt++] = udp.read()
    MESSAGE FORMAT: ~MSG,LEN,[F,I,S],[DATA] 
*/
void uuudeepee() {
  int sized = -1;
  /* IS THERE AN INCOMING UDP PACKET */
  if ( ( sized = udp.parsePacket() ) > 0) {

    /* '~' == 127 MARKS BEGINNING OF MSG */
    if(udp.read() == '~') {
      //char _rBuffer[sized] = { 0 };
      char * _rBuffer = new char[sized - 1];
      udp.read(_rBuffer, sized - 1);
      //just for debug
      //for(int z = 0; z < sized; z++) Serial.print(_rBuffer[z]);
      //Serial.println();

      /* TOKENISE UDP PACKET */
      char* token; 
      vector<string> _t;
      // Get the first token followed by the delimiter ','
      token = strtok(_rBuffer, ",");

      while (token != NULL) { 
        _t.push_back(token);
        // strtok() contains a static pointer to the previous passed string
        token = strtok(NULL, ","); 
      } 
      inudp = _t;
      for(int i=0; i < _t.size();i++)
        Serial.print( _t.at(i).c_str() );
      Serial.println();

      delete _rBuffer;
    }
  }
}

void dof() {

  /* UPDATE IMU READINGS: ACCELERO[3], MAGENTO[3], GYRO[3], 
     YAW, PITCH, ROLL, HEADING */
  accel = imu.getAccelerometer();  
  magnetom = imu.getMagnetometer();
  gyro = imu.getGyrometer();
  yaw = imu.getYaw();
  pitch = imu.getPitch();
  roll = imu.getRoll();
  magnitude = imu.getMagnitude();

  char _buffer[205];
  sprintf(_buffer, "~DOF,F,13,%.7f,%.7f,%.7f,%.7f,%.7f,%.7f,%.7f,%.7f,%.7f,%.7f,%.7f,%.7f,%.7f",  \
          accel[0], accel[1], accel[2], \
          magnetom[0], magnetom[1], magnetom[2], \
          gyro[0], gyro[1], gyro[2], \
          yaw, pitch, roll, magnitude \
  );
  //Serial.println(_buffer);
  if(SELFTEST)
    udp.beginPacket(myArgonIP, LISTEN);
  else
    //udp.beginPacket(argons[4], LISTEN);
      //udp.write( _buffer );
       if (udp.sendPacket(_buffer, sizeof(_buffer), argons[4], LISTEN) < 0) {
        Particle.publish("Error");
      }
    //udp.endPacket();
}

/* PARTICLE CLOUD GOOGLE GEOLATION: LAT, LONG AND ACCURACY - CALLBACK */
void locationCallback(float lat, float lon, float accu) {

  disconnectFromParticleCloud();

  char _buffer[54];
  sprintf(_buffer, "~GEO,F,3,%.7f,%.7f,%.7f", lat, lon, accu);
  Serial.println(_buffer);

  if(SELFTEST)
    udp.beginPacket(myArgonIP, LISTEN);
  else
    //udp.beginPacket(argons[4], LISTEN);
      //udp.write( _buffer );

       if (udp.sendPacket(_buffer, sizeof(_buffer), argons[4], LISTEN) < 0) {
        Particle.publish("Error");
      }
    //udp.endPacket();
}