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


// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd(0);

//LiquidCrystal

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
  ,LIQUIDCRYSTAL_OUT(lcd,{0,0,16,2})
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

void _nav() {
	nav.poll();
}

void setup ()
{
//	pinMode(13, OUTPUT);
//	pinMode(5, OUTPUT);
	//	while (!Serial);
	Serial.begin (115200);
	lcd.begin(16, 2);
	lcd.setBacklight(HIGH);
	delay(100);
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

	Timer1.initialize(20000);
	Timer1.attachInterrupt(_nav); // to run every 10ms seconds

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
	case idleStart: break;
	case idling:
		o.setCursor(0, 0); o.print(F("Water Meter v1.0"));
		break;
	case idleEnd:
		o.print(F("resuming menu."));
		break;
	}
	return proceed;
}
