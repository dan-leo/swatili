/*
 * main.ino
 *
 *  Created on: 22 Apr 2018
 *      Author: d7rob
 */


#include <Arduino.h>

#include "swatili.h"

float k = 1;
unsigned long pulseCount = 0;
unsigned long milliLitres = 0;
unsigned long litreLimit = 1;

volatile byte led_state = LOW;

boolean pulseProcessedFlag = true;
boolean menuIdling = false;
boolean valveOpen = false;


// Create a DS1302 object.
//DS1302 rtc(kCePin, kIoPin, kSclkPin);

Button btBack(buttonPins[0], true);
Button btDown(buttonPins[1], true);
Button btUp(buttonPins[2], true);
Button btEnter(buttonPins[3], true);

Button btReset(buttonPins[4], true);
Button btValve(buttonPins[5], true);

void process_pulse();


//##################################### menu #####################################

enum buttons_in read_buttons();
void navigation(enum buttons_in button);
void idleRefresh();

// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd(0x0);
LCD_idler lcdi;

serialIn serial(Serial);
menuIn* inputsList[]={&serial}; // more inputs can be added here.
chainStream<1> in(inputsList); // 1 is the current number of inputs

result idle(menuOut& o,idleEvent e);

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,FIELD(pulseCount,"Pulses","",0,0,0,0,doNothing,updateEvent,wrapStyle)
  ,FIELD(milliLitres,"Volume","ml",0,0,0,0,doNothing,updateEvent,wrapStyle)
  ,FIELD(litreLimit,"Limit","l",0,50,10,1,doNothing,updateEvent,wrapStyle)
  ,EXIT("<Back")
);

#define MAX_DEPTH 4
MENU_OUTPUTS(out, MAX_DEPTH
  ,LCD_OUT(lcd,{0,0,16,2})
  ,SERIAL_OUT(Serial)
  ,NONE
);
NAVROOT(nav,mainMenu,MAX_DEPTH,in,out); //the navigation root object

//##################################### end menu #####################################

// 1150 ml @ 1007 pulses. 2.25 ml per pulse.

void setup ()
{
	Serial.begin(115200);
	lcd.begin(16, 2);
	lcd.setBacklight(HIGH);
	lcd.setCursor(0, 0); lcd.println(F("Water Meter v0.1"));
	lcd.setCursor(0, 1); lcd.println(F(" Andre van Dyk"));
	delay(500);

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(valveControlPin, OUTPUT);
	pinMode(pulseInterruptPin, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(pulseInterruptPin), process_pulse, FALLING);

	nav.idleTask=idle;//point a function to be used when menu is suspended
	mainMenu[0].enabled = disabledStatus;
	mainMenu[1].enabled = disabledStatus;
	nav.showTitle=false;

	nav.doNav(escCmd); nav.poll();

}  // end of setup

void loop ()
{
	static int _solenoid_state = 0,
			_50ms = millis(),
			_20ms = millis();

	digitalWrite(LED_BUILTIN, led_state);

	if (pulseProcessedFlag) {

		pulseProcessedFlag = false;
	}

	if (!btReset.check()) {
		pulseCount = 0;
		milliLitres = 0;
	}

	if (!btValve.check()) {
		valveOpen = !valveOpen;
		digitalWrite(valveControlPin, valveOpen);
	}

	if (milliLitres > litreLimit * 1000) {
		valveOpen = false;
		digitalWrite(valveControlPin, valveOpen);
	}

	if (millis() - _20ms > 20) {
		navigation(read_buttons());
		nav.poll();
		_20ms = millis();
	}

	if (millis() - _50ms > 50) {
		idleRefresh();
		_50ms = millis();
	}
}  // end of loop

result idle(menuOut& o,idleEvent e) {
	switch(e) {
	    case idleStart: menuIdling = true; break;
	    case idling: /*o.setCursor(0, 0); o.println(F("Water Meter v0.1"));*/ break;
	    case idleEnd: menuIdling = false; break;
	}
	return proceed;
}

void process_pulse() {
  pulseCount++;
  milliLitres = pulseCount * 2.25 * k;
  led_state = !led_state;
  pulseProcessedFlag = true;
}

enum buttons_in read_buttons() {
	if      (btEnter.check() == LOW) return bt_enter;
	else if (btBack.check()  == LOW) return bt_back;
	else if (btUp.check()    == LOW) return bt_incr;
	else if (btDown.check()  == LOW) return bt_decr;
	return bt_none;
}

void navigation(enum buttons_in button) {
	if      (button == bt_enter) nav.doNav(enterCmd);
	else if (button == bt_back)  nav.doNav(escCmd  );
	else if (button == bt_incr)  nav.doNav(upCmd   );
	else if (button == bt_decr)  nav.doNav(downCmd );
}

void idleRefresh() {
	if (menuIdling) {

		for (int i = 0; i < ROWS; i++) {
			lcdi.buf[i].flush();
		}

		lcdi.buf[0] << F("") << milliLitres << F("ml ");
		lcdi.buf[0] << F("") << pulseCount << F(" ");
		lcdi.buf[1] << F("") << valveOpen << F(" ");
		lcdi.buf[1] << F("") << litreLimit << F("l ");

		for (int i = 0; i < ROWS; i++) {
			while (lcdi.buf[i].current_length() < COLS) lcdi.buf[i] << ' ';
			lcd.setCursor(0, i);
			lcd.print(lcdi.buf[i]);
		}
	}
}
