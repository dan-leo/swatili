/*
 * main.ino
 *
 *  Created on: 22 Apr 2018
 *      Author: d7rob
 */


#include <Arduino.h>

#include "setup.h"

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


const byte LCD_RS = 44;
const byte LCD_E =  45;
const byte LCD_D4 = 46;
const byte LCD_D5 = 47;
const byte LCD_D6 = 48;
const byte LCD_D7 = 49;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

serialIn serial(Serial);
menuIn* inputsList[]={&serial}; // more inputs can be added here.
chainStream<1> in(inputsList); // 1 is the current number of inputs

result idle(menuOut& o,idleEvent e);

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,FIELD(pulses_register_count,"Pulses","ml",0,0,0,0,doNothing,updateEvent,wrapStyle)
  ,EXIT("<Back")
);


#define MAX_DEPTH 4

MENU_OUTPUTS(out, MAX_DEPTH
  ,LIQUIDCRYSTAL_OUT(lcd,{0,0,20,4})
  ,SERIAL_OUT(Serial)
  ,NONE
);
NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);//the navigation root object

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


	nav.idleTask=idle;//point a function to be used when menu is suspended
	//  mainMenu[1].enabled=disabledStatus;
	nav.showTitle=false;

	nav.doNav(escCmd); nav.poll();

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

result idle(menuOut& o,idleEvent e) {
	switch(e) {
	case idleStart: o.print(F("suspending menu!")); break;
	case idling:
		o.setCursor(0, 0); o.print(F("Water Meter v1.0"));
		break;
	case idleEnd:
		o.print(F("resuming menu."));
		break;
	}
	return proceed;
}
