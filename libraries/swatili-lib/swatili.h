/*
 * setup.h
 *
 *  Created on: 13 Sep 2017
 *      Author: d7rob
 */

#ifndef SETUP_H_
#define SETUP_H_

//#define DEBUG
//#define ARDUINO_STREAMING

#include "Arduino.h"

//#include <wiring_private.h>
//#include <stdio.h>

#include <menu.h>//menu macros and objects
//#include <menuIO/liquidCrystalOut.h>
#include <menuIO/lcdOut.h>// crowtails lcd menu output
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
using namespace Menu;

#include <Wire.h>
#include <LiquidCrystal_I2C_crowtail.h>
#include <DS1302.h>
#include <Streaming.h>
#include <button.h>
#include <BufferSerial.h>

#define pulseInterruptPin 2
#define valveControlPin 3
const byte buttonPins[] = {A0, A1, A2, 8, 9, 10, 11, 12};

typedef enum buttons_in {
	bt_none  = 0b0,
	bt_back  = 0b1,
	bt_enter = 0b10,
	bt_incr  = 0b100,
	bt_decr  = 0b1000
} buttons_in;

#define ROWS 2
#define COLS 16

class LCD_idler {
private:
	uint8_t tx_buffer[2][COLS + 1];
public:
	BufferSerial buf[2];
	LCD_idler()
	: buf{
		BufferSerial(tx_buffer[0], COLS + 1),
		BufferSerial(tx_buffer[1], COLS + 1),
	} {}
};
extern LCD_idler lcdi;


// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
//const int kCePin   = 8;  // Chip Enable
//const int kIoPin   = 9;  // Input/Output
//const int kSclkPin = 10;  // Serial Clock

//void printTime();
//void setup_rtc();

//void setup_analog_comparator();
//boolean service_trigger();
//void analog_read(uint8_t analog_pin);

#endif /* SETUP_H_ */
