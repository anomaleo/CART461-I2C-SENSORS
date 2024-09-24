/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(AUTOMATIC);

#include <lp55231.h>
Lp55231 ledChip(0x35); 

int channel[] = {2,1,7,3,4,8,5,6,9};

void setup()
{
  Serial.begin(115200);
  waitFor(Serial.isConnected, 5000);
Wire.begin(); 
  ledChip.Begin();
  ledChip.Enable();
  // The LP55231 charge pump should be disabled to avoid instability
  ledChip.SetChargePumpMode(CP_BYPASS);

  // Give things half a sec to settle
  delay(500);

  // Configure each LED channel for logarithmic brightness control
  for(uint8_t i = 0; i < 9; i++)
  {
    ledChip.SetLogBrightness(i, true);
  }

  Serial.println("-- Setup() Complete --");

}


// Flash each LED in turn and then ramp all LEDs from 0 to full brightness
void loop() {

  for(int c=0; c<9; c++){
    ledChip.SetChannelPWM(channel[c]-1, 200);
    delay(200);
    ledChip.SetChannelPWM(channel[c]-1, 0);
  }

  for(int p=0; p<255; p++){
    for(int c=0; c<9; c++){
     ledChip.SetChannelPWM(c, p);
    }
  }

  for(int p=0; p<255; p++){
    for(int c=0; c<9; c++){
     ledChip.SetChannelPWM(c, 255-p);
    }
  }  

  for(int c=0; c<9; c++){
   ledChip.SetChannelPWM(c, 0);
  }

  delay(200);

}