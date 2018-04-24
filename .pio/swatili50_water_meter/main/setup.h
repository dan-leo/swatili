/*
 * setup.h
 *
 *  Created on: 13 Sep 2017
 *      Author: d7rob
 */

#ifndef SETUP_H_
#define SETUP_H_

#include "Arduino.h"
#include <wiring_private.h>

#include <stdio.h>
#include <DS1302.h>


#include <menu.h>
#include <menuIO/liquidCrystalOut.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
using namespace Menu;

#include <LiquidCrystal.h>

//#include "A4988.h"


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
