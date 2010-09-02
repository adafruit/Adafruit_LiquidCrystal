// Code by Adafruit Industries/Limor Fried
// License: LGPL

#include <Wire.h>
#include <avr/pgmspace.h>
#include "MCP23008.h"
#include <WProgram.h>


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
  Wire.send(MCP23008_IODIR);	
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
  iodir = Wire.receive();

  // set the pin and direction
  if (d == INPUT) {
    iodir |= 1 << p; 
  } else {
    iodir &= ~(1 << p);
  }

  // write the new IODIR
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
  Wire.send(MCP23008_IODIR);
  Wire.send(iodir);	
  Wire.endTransmission();
}


void MCP23008::digitalWrite(uint8_t p, uint8_t d) {
  uint8_t gpio;
  
  // only 8 bits!
  if (p > 7)
    return;

  // read the current GPIO output latches
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
  Wire.send(MCP23008_OLAT);	
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
  gpio = Wire.receive();

  // set the pin and direction
  if (d == HIGH) {
    gpio |= 1 << p; 
  } else {
    gpio &= ~(1 << p);
  }

  // write the new GPIO
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
  Wire.send(MCP23008_GPIO);
  Wire.send(gpio);	
  Wire.endTransmission();
}

void MCP23008::pullUp(uint8_t p, uint8_t d) {
  uint8_t gppu;
  
  // only 8 bits!
  if (p > 7)
    return;

  // read the current pullup resistor set
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
  Wire.send(MCP23008_GPPU);	
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
  gppu = Wire.receive();

  // set the pin and direction
  if (d == HIGH) {
    gppu |= 1 << p; 
  } else {
    gppu &= ~(1 << p);
  }

  // write the new GPIO
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
  Wire.send(MCP23008_GPPU);
  Wire.send(gppu);	
  Wire.endTransmission();
}

uint8_t MCP23008::digitalRead(uint8_t p) {
  // only 8 bits!
  if (p > 7)
    return 0;

  // read the current GPIO
  Wire.beginTransmission(MCP23008_ADDRESS | i2caddr);
  Wire.send(MCP23008_GPIO);	
  Wire.endTransmission();
  
  Wire.requestFrom(MCP23008_ADDRESS | i2caddr, 1);
  return (Wire.receive() >> p) & 0x1;
}
