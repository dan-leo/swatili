/*
 * main.ino
 *
 *  Created on: 22 Apr 2018
 *      Author: d7rob
 */


#include <Arduino.h>

// #include "setup.h"

volatile boolean triggered;
unsigned long pulse_count = 0;
unsigned long pulses_register_count = 0;
int k_pulse_to_litre = 10;

const byte interruptPin = 2;
volatile byte state = LOW;
const byte ledPin = 13;

boolean pulse_processed = true;


// Create a DS1302 object.
//DS1302 rtc(kCePin, kIoPin, kSclkPin);

//ISR (ANALOG_COMP_vect)
//{
//	triggered = true;
//}

void process_pulse() {
  pulses_register_count++;
  state = !state;
  pulse_processed = true;
}

// 1150 ml @ 1007 pulses. 2.25 ml per pulse.

void setup ()
{
//	pinMode(13, OUTPUT);
//	pinMode(5, OUTPUT);
	//	while (!Serial);
	Serial.begin (115200);
//	Serial.println ("Started.");
//	setup_analog_comparator();
//	setup_rtc();
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(3, INPUT_PULLUP);
	pinMode(interruptPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(interruptPin), process_pulse, CHANGE);

}  // end of setup

void loop ()
{
	static int _solenoid_state = 0;

	digitalWrite(ledPin, state);

	if (pulse_processed) {
		Serial.println(pulses_register_count);
		pulse_processed = false;
	}

	if (!digitalRead(3)) {
		pulses_register_count = 0;
	}

//	if (service_trigger()) {
////		if (!(pulse_count%3)) {
////			_solenoid_state = !_solenoid_state;
////			digitalWrite(5, _solenoid_state);
////		}
//		if (!(pulse_count % 20)) {
//			printTime();
//			Serial.println(k_pulse_to_litre * pulses_register_count++);
//		}
//	}
}  // end of loop
