/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/eo/Desktop/CART461/CART461-I2C-SENSORS/PUBLISH-SUBSCRIBE/PUBLISH/src/PUBLISH.ino"
/*
 * Project PUBLISH AND SUBSCRIBE
 * Description:
 * Author: me
 * Date: 
 */

#include <Particle.h>
#include <vector>
void setup();
void loop();
void backtome(const char *event, const char *data);
#line 10 "/Users/eo/Desktop/CART461/CART461-I2C-SENSORS/PUBLISH-SUBSCRIBE/PUBLISH/src/PUBLISH.ino"
using namespace std;

/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* ALWAYS RUN PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */ 
SYSTEM_THREAD(ENABLED);

/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC); // DONE BY DEFAULT
//SYSTEM_MODE(SEMI_AUTOMATIC); // CONTROL WHEN & HOW TO CONNECT TO PARTICLE CLOUD (WIFI RADIO ON)
//SYSTEM_MODE(MANUAL); // CONTROL WHEN & HOW TO CONNECT TO WIFI NETWORK

#define DEBUG_LED D7 // SMALL BLUE LED NEXT USB CONNECTOR (RIGHT OF USB)
#define WHITE_LED D2 // GOOD OL' LED
#define R_LED     D6 // RED LED
#define G_LED     D5 // GREEN LED
#define B_LED     D4 // BLUE LED
#define LDR       A0 // ANALOG_IN = ( 0 ... 4095 ) => 2^12 bits.
#define B_TN      D0 // MOMENTARY BUTTON

void setup() {

  /* PARTICLE CLOUD INTENTIONS ALWAYS DECLARED IN VOID SETUP */
  /* 
    PARTICLE FUNCTIONS 'n' VARIABLES ARE SPECIAL CLOUD API OPERATIONS - 
    MORE ABOUT FUNCTIONS AND VARIABLES IN A FUTURE WORKSHOP (#3). 
  */
  Particle.variable("location", "ME @ MTL", STRING);

  /* PARTICLE PUBLISH 'n' SUBSCRIBE ARE MULTICAST OPERATIONS - FULLY CONNECTED 
    SIMPLY SUBSCRIBE TO A KNOWN DATA SOURCE OR PUBLISH DATA AS SOURCE
    THE ONLY CAVEAT - ONE (1) MSG PER SECOND OVER THE CLOUD 
  */
  Particle.subscribe("refertome", backtome);

  Serial.begin(57600);
  //while( !Serial.isConnected() ) // wait for Host to open serial port
  waitFor(Serial.isConnected, 15000); 

  /* BLUE DEBUG LED */
  pinMode(DEBUG_LED, OUTPUT);
  /* WHITE LED ? DIGITAL : PWM */
  pinMode(WHITE_LED, OUTPUT);

  /* RGB LED */
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);

  /* INPUT: PUSH BUTTON */
  pinMode(B_TN, INPUT_PULLUP); // REQUIRE PULLUP OR PULLDOWN RESISTOR

  delay(5); // Force Serial.println in void setup()
  Serial.println("Completed void setup");
}

void loop() {

    if( digitalRead(B_TN) == LOW ) {
        /* EVENTS CAN ONLY BE PUBLISHED ONCE PER SECOND */
        digitalWrite(DEBUG_LED, HIGH); 

        // https://docs.particle.io/reference/device-os/api/cloud-functions/particle-publish/
        if( Particle.publish("refertome", "45.4786288,-73.617024,45.4953688,-73.57799640000002") ) Serial.println ("published");
        delay(1001); // PUBLISH ONCE PER SECOND

        digitalWrite(DEBUG_LED, LOW);
    }
}

/* PARTICLE CLOUD SUBSCRIBE CALLBACK */
void backtome(const char *event, const char *data) {
/* 
  Particle.subscribe handlers are void functions. They take two variables the name of your event, 
  and any data (UTF-8) that goes along with your event.
  */

 /* EXAMPLE TOKENISER */
    //char *datas = "45.4786288,-73.617024,45.4953688,-73.57799640000002";
     vector<string> result;
      //get original length
    char copyData[strlen(data)];
    strcpy(copyData, data);

    char* token; 

    //get a token 
    token = strtok(copyData, ",");

    while (token != NULL) { 
    //add to the result array
      result.push_back(token);
      // strtok() contains a static pointer to the previous passed string
      token = strtok(NULL, ","); 
    } 

    for(int i=0; i<result.size();i++){
        Serial.println( result.at(i).c_str() );
    }
    Serial.println("received");
}