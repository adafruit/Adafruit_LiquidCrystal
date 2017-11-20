#include "Arduino.h"
#include "Adafruit_LiquidCrystal.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

// FOR Arduino Due
#if !defined(_BV)
#	define _BV(bit) (1 << (bit))
#endif 

/* i2C code:
 * pinout:
 * _rs_pin = 1;
 * _rw_pin = 255;
 * _enable_pin = 2;
 * _data_pins[0] = 3;  // really d4
 * _data_pins[1] = 4;  // really d5
 * _data_pins[2] = 5;  // really d6
 * _data_pins[3] = 6;  // really d7
 *
 * because we're only using a very limited subset of the 23008's
 * functionality, let's just simplify...
 */
#define LCD_GPIO (0x09)
#define LCD_IODIR (0x00)
#define LCD_RS (0x02)
#define LCD_ENABLE (0x04)
#define LCD_BACKLIGHT (0x80)
#define LCD_DATA (0x0F << 3)

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// Adafruit_LiquidCrystal constructor is called).

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
			     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t rs, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
			     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t rs,  uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t i2caddr) {
  // MCP23008 is base address 20, it has three address pins.
  _i2cAddr = 0x20 | (i2caddr & 7);
  // we don't need to set any of the pin values, as they're fixed and
  // we don't need to use them

  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
}


Adafruit_LiquidCrystal::Adafruit_LiquidCrystal(uint8_t data, uint8_t clock, uint8_t latch ) {
  _i2cAddr = 255;

  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  // the SPI expander pinout
  _rs_pin = 1;
  _rw_pin = 255;
  _enable_pin = 2;
  _data_pins[0] = 6;  // really d4
  _data_pins[1] = 5;  // really d5
  _data_pins[2] = 4;  // really d6
  _data_pins[3] = 3;  // really d7
  
  _SPIdata = data;
  _SPIclock = clock;
  _SPIlatch = latch;

  _SPIbuff = 0;

  // we can't begin() yet :(
}


void Adafruit_LiquidCrystal::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
			 uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
			 uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  _rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;
  
  _data_pins[0] = d0;
  _data_pins[1] = d1;
  _data_pins[2] = d2;
  _data_pins[3] = d3; 
  _data_pins[4] = d4;
  _data_pins[5] = d5;
  _data_pins[6] = d6;
  _data_pins[7] = d7; 

  _i2cAddr = 255;
  _SPIclock = _SPIdata = _SPIlatch = 255;

  if (fourbitmode)
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  else 
    _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
}

void Adafruit_LiquidCrystal::i2c_setpins(uint8_t pins) {
    Wire.beginTransmission(_i2cAddr);
    Wire.write((byte) LCD_GPIO);
    Wire.write((byte) pins);
    Wire.endTransmission();
}

void Adafruit_LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  // check if i2c
  if (_i2cAddr != 255) {
    _i2cGPIO = 0x00;
    Wire.begin();
    Wire.beginTransmission(_i2cAddr);
    Wire.write((byte) LCD_IODIR);
    // 0 == output, we want everything but pin0 set as an output:
    Wire.write((byte) 0x01);
    // and zero out the other regs
    Wire.write((byte) 0x00);
    Wire.write((byte) 0x00);
    Wire.write((byte) 0x00);
    Wire.write((byte) 0x00);
    Wire.write((byte) 0x00);
    Wire.write((byte) 0x00);
    Wire.write((byte) 0x00);
    Wire.write((byte) 0x00);
    Wire.write((byte) 0x00);
    Wire.endTransmission();

    _i2cGPIO |= LCD_BACKLIGHT;
    i2c_setpins(_i2cGPIO);
  } else if (_SPIclock != 255) {
    pinMode(_SPIdata, OUTPUT);
    pinMode(_SPIclock, OUTPUT);
    pinMode(_SPIlatch, OUTPUT);
    _SPIbuff = 0x80; // backlight
  } else {
    pinMode(_rs_pin, OUTPUT);
    // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
    if (_rw_pin != 255) { 
      pinMode(_rw_pin, OUTPUT);
    }
    pinMode(_enable_pin, OUTPUT);
    
  }

  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _currline = 0;

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != 0) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
  delayMicroseconds(50000); 
  // Now we pull both RS and R/W low to begin commands
  // for i2c mode, we set those low in the initial pin settings.
  if (_i2cAddr == 255) {
    _digitalWrite(_rs_pin, LOW);
    _digitalWrite(_enable_pin, LOW);
  }
  if (_rw_pin != 255) { 
    _digitalWrite(_rw_pin, LOW);
  }
  
  //put the LCD into 4 bit or 8 bit mode
  if (! (_displayfunction & LCD_8BITMODE)) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms
    
    // third go!
    write4bits(0x03); 
    delayMicroseconds(150);

    // finally, set to 8-bit interface
    write4bits(0x02); 
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1ms

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(150);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

}

/********** high level commands, for the user! */
void Adafruit_LiquidCrystal::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void Adafruit_LiquidCrystal::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void Adafruit_LiquidCrystal::setCursor(uint8_t col, uint8_t row)
{
  row = row % _numlines;
  row = ((row % 2) * 0x40) + (((row >> 1) % 2) * 0x14);
  
  command(LCD_SETDDRAMADDR | (col + row));
}

// Turn the display on/off (quickly)
void Adafruit_LiquidCrystal::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_LiquidCrystal::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void Adafruit_LiquidCrystal::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_LiquidCrystal::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void Adafruit_LiquidCrystal::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_LiquidCrystal::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void Adafruit_LiquidCrystal::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void Adafruit_LiquidCrystal::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void Adafruit_LiquidCrystal::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void Adafruit_LiquidCrystal::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void Adafruit_LiquidCrystal::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void Adafruit_LiquidCrystal::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void Adafruit_LiquidCrystal::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void Adafruit_LiquidCrystal::command(uint8_t value) {
  send(value, LOW);
}

#if ARDUINO >= 100
inline size_t Adafruit_LiquidCrystal::write(uint8_t value) {
  send(value, HIGH);
  return 1;
}
#else
inline void Adafruit_LiquidCrystal::write(uint8_t value) {
  send(value, HIGH);
}
#endif

/************ low level data pushing commands **********/

// little wrapper for i/o writes
void  Adafruit_LiquidCrystal::_digitalWrite(uint8_t p, uint8_t d) {
  if (_i2cAddr != 255) {
    // do nothing, we should never be called in the i2c case
  } else if (_SPIclock != 255) {
    if (d == HIGH)
      _SPIbuff |= (1 << p);
    else 
      _SPIbuff &= ~(1 << p);

    digitalWrite(_SPIlatch, LOW);
    shiftOut(_SPIdata, _SPIclock, MSBFIRST,_SPIbuff);
    digitalWrite(_SPIlatch, HIGH);
  } else {
    // straightup IO
    digitalWrite(p, d);
  }
}

// Allows to set the backlight, if the LCD backpack is used
void Adafruit_LiquidCrystal::setBacklight(uint8_t status) {
  // check if i2c or SPI
  if (_i2cAddr != 255) {
    if (status) {
      _i2cGPIO |= LCD_BACKLIGHT;
    } else {
      _i2cGPIO &= ~LCD_BACKLIGHT;
    }
    i2c_setpins(_i2cGPIO);
  } else if (_SPIclock != 255) {
    _digitalWrite(7, status); // backlight is on pin 7
  }
}

// little wrapper for i/o directions
void  Adafruit_LiquidCrystal::_pinMode(uint8_t p, uint8_t d) {
  if (_i2cAddr != 255) {
    // nothing to do, we set them correctly at startup
  } else if (_SPIclock != 255) {
    // nothing!
  } else {
    // straightup IO
    pinMode(p, d);
  }
}

// write either command or data, with automatic 4/8-bit selection
void Adafruit_LiquidCrystal::send(uint8_t value, boolean mode) {
  if (_i2cAddr != 255) {
    if (mode) {
      _i2cGPIO |= LCD_RS;
    } else {
      _i2cGPIO &= ~LCD_RS;
    }
  } else {
    _digitalWrite(_rs_pin, mode);

    // if there is a RW pin indicated, set it low to Write
    if (_rw_pin != 255) { 
      _digitalWrite(_rw_pin, LOW);
    }
  }
  if (_displayfunction & LCD_8BITMODE) {
    write8bits(value); 
  } else {
    write4bits(value>>4);
    write4bits(value);
  }
}

void Adafruit_LiquidCrystal::pulseEnable(void) {
  _digitalWrite(_enable_pin, LOW);
  delayMicroseconds(1);    
  _digitalWrite(_enable_pin, HIGH);
  delayMicroseconds(1);    // enable pulse must be >450ns
  _digitalWrite(_enable_pin, LOW);
  delayMicroseconds(100);   // commands need > 37us to settle
}

void Adafruit_LiquidCrystal::write4bits(uint8_t value) {
  if (_i2cAddr != 255) {
    _i2cGPIO = (_i2cGPIO & ~LCD_DATA) | ((value & 0xF) << 3);
    i2c_setpins(_i2cGPIO);
    i2c_setpins(_i2cGPIO | LCD_ENABLE);
    i2c_setpins(_i2cGPIO);
  } else {
    for (int i = 0; i < 4; i++) {
      _pinMode(_data_pins[i], OUTPUT);
      _digitalWrite(_data_pins[i], (value >> i) & 0x01);
    }
    pulseEnable();
  }
}

void Adafruit_LiquidCrystal::write8bits(uint8_t value) {
  for (int i = 0; i < 8; i++) {
    _pinMode(_data_pins[i], OUTPUT);
    _digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }
  
  pulseEnable();
}
