#ifndef _ADAFRUIT_I2C_REGISTER_H_
#define _ADAFRUIT_I2C_REGISTER_H_

#include <Adafruit_BusIO_Register.h>

#if defined(PARTICLE)
  #include <Particle.h>
  #include <Arduino.h>
#elif defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #include "Print.h"
#else
  #include "WProgram.h"
#endif


typedef Adafruit_BusIO_Register Adafruit_I2CRegister;
typedef Adafruit_BusIO_RegisterBits Adafruit_I2CRegisterBits;

#endif
