// Code by Adafruit Industries/Limor Fried
// License: LGPL

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include "Wire.h"
#include <avr/pgmspace.h>
#include "MCP23008.h"


////////////////////////////////////////////////////////////////////////////////
// RTC_DS1307 implementation

void MCP23008::begin(uint8_t addr) {
  if (addr > 7) {
    addr = 7;
  }
  i2caddr = addr;

  Wire.begin();

  // set defaults!
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
#if ARDUINO >= 100
  Wire.write((byte)MCP23008_IODIR);
  Wire.write((byte)0xFF);  // all inputs
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);
  Wire.write((byte)0x00);	
#else
  Wire.send(MCP23008_IODIR);
  Wire.send(0xFF);  // all inputs
  Wire.send(0x00);
  Wire.send(0x00);
  Wire.send(0x00);
  Wire.send(0x00);
  Wire.send(0x00);
  Wire.send(0x00);
  Wire.send(0x00);
  Wire.send(0x00);
  Wire.send(0x00);	
#endif
  Wire.endTransmission();

}

void MCP23008::begin(void) {
  begin(0);
}

void MCP23008::pinMode(uint8_t p, uint8_t d) {
  uint8_t iodir;
  

  // only 8 bits!
  if (p > 7)
    return;

  // read the current IODIR
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
#if ARDUINO >= 100
  Wire.write((byte)MCP23008_IODIR);	
#else
  Wire.send(MCP23008_IODIR);	
#endif
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
#if ARDUINO >= 100
  iodir = Wire.read();
#else
  iodir = Wire.receive();
#endif

  // set the pin and direction
  if (d == INPUT) {
    iodir |= 1 << p; 
  } else {
    iodir &= ~(1 << p);
  }

  // write the new IODIR
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
#if ARDUINO >= 100
  Wire.write((byte)MCP23008_IODIR);
  Wire.write(iodir);	
#else
  Wire.send(MCP23008_IODIR);
  Wire.send(iodir);	
#endif
  Wire.endTransmission();
}


void MCP23008::digitalWrite(uint8_t p, uint8_t d) {
  uint8_t gpio;
  
  // only 8 bits!
  if (p > 7)
    return;

  // read the current GPIO output latches
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
#if ARDUINO >= 100
  Wire.write((byte)MCP23008_OLAT);	
#else
  Wire.send(MCP23008_OLAT);	
#endif
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
#if ARDUINO >= 100
  gpio = Wire.read();
#else
  gpio = Wire.receive();
#endif

  // set the pin and direction
  if (d == HIGH) {
    gpio |= 1 << p; 
  } else {
    gpio &= ~(1 << p);
  }

  // write the new GPIO
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
#if ARDUINO >= 100
  Wire.write((byte)MCP23008_GPIO);
  Wire.write((byte)gpio);	
#else
  Wire.send(MCP23008_GPIO);
  Wire.send(gpio);	
#endif
  Wire.endTransmission();
}

void MCP23008::pullUp(uint8_t p, uint8_t d) {
  uint8_t gppu;
  
  // only 8 bits!
  if (p > 7)
    return;

  // read the current pullup resistor set
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
#if ARDUINO >= 100
  Wire.write((byte)MCP23008_GPPU);	
#else
  Wire.send(MCP23008_GPPU);	
#endif
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
#if ARDUINO >= 100
  gppu = Wire.read();
#else
  gppu = Wire.receive();
#endif

  // set the pin and direction
  if (d == HIGH) {
    gppu |= 1 << p; 
  } else {
    gppu &= ~(1 << p);
  }

  // write the new GPIO
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
#if ARDUINO >= 100
  Wire.write((byte)MCP23008_GPPU);
  Wire.write((byte)gppu);	
#else
  Wire.send(MCP23008_GPPU);
  Wire.send(gppu);	
#endif
  Wire.endTransmission();
}

uint8_t MCP23008::digitalRead(uint8_t p) {
  // only 8 bits!
  if (p > 7)
    return 0;

  // read the current GPIO
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
#if ARDUINO >= 100
  Wire.write((byte)MCP23008_GPIO);	
#else
  Wire.send(MCP23008_GPIO);	
#endif
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
#if ARDUINO >= 100
  return (Wire.read() >> p) & 0x1;
#else
  return (Wire.receive() >> p) & 0x1;
#endif
}
