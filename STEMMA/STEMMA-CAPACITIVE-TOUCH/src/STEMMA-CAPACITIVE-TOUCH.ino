#include <Particle.h>
/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(MANUAL);


#include <Adafruit_MPR121.h>
#include <dotstar.h>

#define NUMPIXELS 30 // Number of LEDs in strip

//-------------------------------------------------------------------
// NOTE: If you find that the colors you choose are not correct,
// there is an optional 2nd argument (for HW SPI) and
// 4th arg. (for SW SPI) that you may specify to correct the colors.
//-------------------------------------------------------------------
// e.g. Adafruit_DotStar(NUMPIXELS, DOTSTAR_BGR);
// e.g. Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);
//
// DOTSTAR_RGB
// DOTSTAR_RBG
// DOTSTAR_GRB
// DOTSTAR_GBR
// DOTSTAR_BRG
// DOTSTAR_BGR (default)

#if (PLATFORM_ID == 32) // P2/Photon2
//-------------------------------------------------------------------
// P2/Photon2 must use dedicated SPI Interface API (Hardware SPI/SPI1):
// e.g. Adafruit_DotStar(NUMPIXELS, SPI_INTERFACE, DOTSTAR_BGR);
//-------------------------------------------------------------------
// SPI: MO (data), SCK (clock)
#define SPI_INTERFACE SPI
// SPI1: D2 (data), D4 (clock)
// #define SPI_INTERFACE SPI1
Adafruit_DotStar strip(NUMPIXELS, SPI_INTERFACE, DOTSTAR_BGR);

#else // Argon, Boron, etc..
//-------------------------------------------------------------------
// Here's how to control the LEDs from any two pins (Software SPI):
// e.g. Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);
//-------------------------------------------------------------------
#define DATAPIN   MOSI
#define CLOCKPIN  SCK
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

//-------------------------------------------------------------------
// Here's how to control the LEDs from SPI pins (Hardware SPI):
// e.g. Adafruit_DotStar(NUMPIXELS, DOTSTAR_RGB);
//-------------------------------------------------------------------
// Hardware SPI is a little faster, but must be wired to specific pins
// (Core/Photon/P1/Electron = pin A5 for data, A3 for clock)
//Adafruit_DotStar strip(NUMPIXELS, DOTSTAR_BGR);

#endif // #if (PLATFORM_ID == 32)

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
}

void loop() {
  // Get the currently touched pads
  currtouched = cap.touched();
  
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
    }
  }

  // reset our state
  lasttouched = currtouched;

  // comment out this line for detailed data from the sensor!
  return;
  
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
  
  // put a delay so it isn't overwhelming
  delay(100);
}