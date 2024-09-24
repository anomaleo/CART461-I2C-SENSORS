#include <Particle.h>
/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC);

#include "SparkFun_GridEYE_Arduino_Library.h"
GridEYE grideye;

#include "SparkFun_SGP30_Arduino_Library.h"
SGP30 mySensor;

/* JSON PARSER AND GENERATOR */
#include "JsonParserGeneratorRK.h"

/* PARTICLE CLOUD GOOGLE MAPS INTEGRATION */
#include "google-maps-device-locator.h"
GoogleMapsDeviceLocator locator;

// Use these values (in degrees C) HEATMAP
#define HOT 40
#define COLD 20
int pixelTable[64]; // GRID-EYE

#define B_TN  D6 // MOMENTARY BUTTON

void calibrateAirQuality() {
 //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb
  int cnt = 0;

  while ( cnt < 16) {
    //measure CO2 and TVOC levels
    mySensor.measureAirQuality();
    Serial.print("CO2: ");
    Serial.print(mySensor.CO2);
    Serial.print(" ppm\tTVOC: ");
    Serial.print(mySensor.TVOC);
    Serial.println(" ppb");
    //get raw values for H2 and Ethanol
    mySensor.measureRawSignals();
    Serial.print("Raw H2: ");
    Serial.print(mySensor.H2);
    Serial.print(" \tRaw Ethanol: ");
    Serial.println(mySensor.ethanol);

    delay(1005);
    cnt++;
  }
}

void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);

  /* INPUT: PUSH BUTTON */
  pinMode(B_TN, INPUT_PULLUP); // NO RESISTOR

  Wire.begin();
  Wire.setClock(400000);

  grideye.begin();

  //SPG30
  if (mySensor.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();

  calibrateAirQuality();

  /* GEOLOCATION: LAT, LONG AND ACCRUACY - GOOGLE MAPS */
  /* GEOLATION: LAT, LONG AND ACCRUACY - GOOGLE MAPS */
  locator.withEventName("cslablocation");
  locator.withSubscribe(voicescarry);

  /* WEBHOOK INTEGRATION RESPONSE  */
  Particle.subscribe("hook-response/mongo-cslab-sgp-grid", mongosays, MY_DEVICES);
}

void mongosays(const char *event, const char *data) {
  // Handle the integration response
  Serial.println("Mongo response: ");
  Serial.println(data);
}

// void getItOut(String msg) {
//   Serial.println(jw.getBuffer());
//   if ( Particle.publish("mongo-cslab-sgp-grid", jw.getBuffer()) ) { Serial.println (msg.c_str()); jw.clear(); }
//   delay(2004); // WAIT A LITTLE FOR POST TO COMPLETE
// }

/* WE ASSUME GOOGLE GEOLOCATION FOR ALL EXAMPLES - EXCEPT TURBIDITY SENSOR ;) */
void buildJSON(float lat = 0.0, float lon = 0.0, float accu = 0.0) {

  JsonWriterStatic<768> jw; {
    JsonWriterAutoObject obj(&jw);
    jw.insertKeyValue("sensor", "sgp30-grid-eye-sensor");
    if (lat == 0.0 && lon == 0.0 && accu == 0.0)  
      jw.insertKeyValue("geolocation", "undefined");
    else 
      jw.insertKeyValue("geolocation", "defined");

    jw.insertKeyValue("device-name", "cslab-methionine");
		jw.insertKeyValue("latitude", lat);
    jw.insertKeyValue("longitude", lon);
    jw.insertKeyValue("accuracy", accu);

    /* SGP30 Air Quality Sensor */
    mySensor.measureAirQuality();
    delay(125);
    jw.insertKeyValue("CO2-PPM", mySensor.CO2);
    jw.insertKeyValue("TVOC-PPB", mySensor.TVOC);

    mySensor.measureRawSignals();
    delay(125);
    jw.insertKeyValue("H2-RAW", mySensor.H2);
    jw.insertKeyValue("ETHANOL-RAW", mySensor.ethanol);

    /* GRID CAPTURE */
    float testPixelValue = 0;
    float hotPixelValue = 0;
    int hotPixelIndex = 0;
    String ge = "";

    for(unsigned char i = 0; i < 64; i++){
      testPixelValue = grideye.getPixelTemperature(i);
      ge += String(testPixelValue, 2);
      if (i < 63) ge += ",";

      if(testPixelValue > hotPixelValue){
        hotPixelValue = testPixelValue;
        hotPixelIndex = i;
      }
    } 
    jw.insertKeyValue("grideye", ge.c_str());
    jw.insertKeyValue("hotpixelindex", hotPixelIndex);
    jw.insertKeyValue("hotpixelvalue", String(hotPixelValue, 2).c_str());

}
  /* TRANSMIT DATA TO A HTTPS SERVER OUT OF PARTICLE CLOUD PLATFORM - USE A WEBHOOK */ 
  //getItOut("BUILD JSON");
  Serial.println(jw.getBuffer());
  if ( Particle.publish("mongo-cslab-sgp-grid", jw.getBuffer()) ) { Serial.println ("SENT JSON"); }
  delay(2004); // WAIT A LITTLE FOR POST TO COMPLETE
  
}


void loop() {
  /* GOOGLE GEOLOCATION LOOP - OPTIONAL (CAN BE REMOVED) */
  //locator.loop(); 

  /* EVENT WHICH INDUCES A GOOGLE GEOLOCATION REQUEST & PARTICLE PUBLISH ROUTINE */
  if( digitalRead(B_TN) == LOW ) {
    /* REQUEST LOCATION FROM GOOGLE */
    delay(250);
    //locator.publishLocation();
    buildJSON();
  } 
}

/* PARTICLE CLOUD GOOGLE GEOLATION: LAT, LONG AND ACCRUACY - CALLBACK */
void voicescarry(float lat, float lon, float accu) {
  buildJSON(lat, lon, accu);
}