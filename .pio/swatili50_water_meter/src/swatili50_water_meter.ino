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
float litreLimit = 0.5;

volatile byte led_state = LOW;

boolean pulseProcessedFlag = true;
boolean menuIdling = false;
boolean valveOpen = false;
//boolean preLitreReset = true;


// Create a DS1302 object.
//extern DS1302 rtc;
extern Time t;
extern Time tReset;

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

result idle(menuOut& o,idleEvent e) {
	switch(e) {
	    case idleStart: menuIdling = true; break;
	    case idling: /*o.setCursor(0, 0); o.println(F("Water Meter v0.1"));*/ break;
	    case idleEnd: menuIdling = false; break;
	}
	return proceed;
}

result resetTime(eventMask e, prompt &item);
result resetTime_alert(menuOut& o,idleEvent e);
result updateTime(eventMask e, navNode& nav, prompt &item) {
	Serial << F("New time set!") << endl;
	setup_rtc(t);
	return proceed;
}

MENU(timeSubmenu,"Time setup",doNothing,anyEvent,noStyle
  ,FIELD(t.yr,"Year","",2000,2099,10,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.mon,"Month","",1,12,1,0,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.date,"Date","",1,31,10,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.hr,"Hours","",0,23,5,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.min,"Minutes","",0,59,10,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.sec,"Seconds","",0,59,10,1,updateTime,exitEvent,wrapStyle)
  ,EXIT("<Back")
);

MENU(resetLitresSubmenu,"T reset litres",doNothing,anyEvent,noStyle
  ,FIELD(tReset.hr,"Hour","",0,23,5,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(tReset.min,"Minute","",0,59,10,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(tReset.sec,"Second","",0,59,10,1,updateTime,exitEvent,wrapStyle)
  ,EXIT("<Back")
);

//MENU(timeCutoffSubmenu,"Time cutoff",doNothing,anyEvent,noStyle
//  ,FIELD(tCutoff.hr,"Hour","",0,23,5,1,updateTime,exitEvent,wrapStyle)
//  ,FIELD(tCutoff.min,"Minute","",0,59,10,1,updateTime,exitEvent,wrapStyle)
//  ,FIELD(tCutoff.sec,"Second","",0,59,10,1,updateTime,exitEvent,wrapStyle)
//  ,EXIT("<Back")
//);

MENU(resetTimeSubmenu,"Reset Time",doNothing,anyEvent,noStyle
  ,OP("Yes",resetTime,enterEvent)
  ,OP("No",doNothing,enterEvent)
  ,EXIT("<Back")
);

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,FIELD(pulseCount,"Pulses","",0,0,0,0,doNothing,updateEvent,wrapStyle)
  ,FIELD(milliLitres,"Volume","ml",0,0,0,0,doNothing,updateEvent,wrapStyle)
  ,FIELD(litreLimit,"Limit","l",0.0,50.0,1,0.1,doNothing,updateEvent,wrapStyle)
  ,FIELD(k,"k constant","",0,3,0.1,0.01,doNothing,updateEvent,wrapStyle)
  ,SUBMENU(timeSubmenu)
  ,SUBMENU(resetLitresSubmenu)
//  ,SUBMENU(timeCutoffSubmenu)
  ,SUBMENU(resetTimeSubmenu)
  ,EXIT("<Back")
);

#define MAX_DEPTH 4
MENU_OUTPUTS(out, MAX_DEPTH
  ,LCD_OUT(lcd,{0,0,16,2})
  ,SERIAL_OUT(Serial)
  ,NONE
);
NAVROOT(nav,mainMenu,MAX_DEPTH,in,out); //the navigation root object

result resetTime(eventMask e, prompt &item) {
	t = Time(2018, 4, 28, 04, 48, 00, Time::kSaturday);
	setup_rtc(t);
	nav.idleOn(resetTime_alert);
	return proceed;
}

result resetTime_alert(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.setCursor(0,0);
    o.print("Time reset.");
    o.setCursor(0,1); o.print("[enter] to cont.");
  }
  return proceed;
}

//##################################### end menu #####################################

// 1150 ml @ 1007 pulses. 2.25 ml per pulse.

void setup ()
{
	Serial.begin(115200);
	lcd.begin(16, 2);
	lcd.setBacklight(HIGH);
	lcd.setCursor(0, 0); lcd.print(F("Water Meter v0.1"));
	lcd.setCursor(0, 1); lcd.print(F(" Andre van Dyk  "));
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
	static unsigned long _solenoid_state = 0,
			//_50ms = millis(),
			//_20ms = millis(),
			_10ms = millis(),
			_100ms = millis(),
			_500ms = millis();

	digitalWrite(LED_BUILTIN, led_state);

	if (pulseProcessedFlag) {
		milliLitres = pulseCount * 2.25 * k;
		pulseProcessedFlag = false;
	}

	if (milliLitres * 1.0 > litreLimit * 1000) {
		valveOpen = false;
		digitalWrite(valveControlPin, valveOpen);
	}

	if (millis() - _10ms > 10) {

		if (!btReset.check()) {
			pulseCount = 0;
			milliLitres = 0;
		}
		if (!btValve.check()) {
			valveOpen = !valveOpen;
			digitalWrite(valveControlPin, valveOpen);
		}

		navigation(read_buttons());
		nav.poll();
		_10ms = millis();
	}

	if (millis() - _100ms > 100) {
		idleRefresh();
		_100ms = millis();
	}

	if (millis() - _500ms > 500) {

//		boolean tSGreaterC = tStart.hr >= tCutoff.hr &&
//				tStart.min >= tCutoff.min &&
//				tStart.sec >= tCutoff.sec;
//
//		boolean tLessThanStart = t.hr <= tStart.hr &&
//				t.min <= tStart.min &&
//				t.sec <= tStart.sec;
//
		if (t.hr == tReset.hr &&
				t.min == tReset.min &&
				t.sec == tReset.sec) {
			pulseCount = 0;
			milliLitres = 0;
		}

//		Serial << tSGreaterC << tLessThanStart << tGreaterThanCutoff << endl;

		// xyz + !x(y + z)
		// !xy + !xz + yz
		// 101

//		if ((tGreaterThanCutoff && tLessThanStart && tGreaterThanCutoff) ||
//			(!tSGreaterC && !tLessThanStart && !tGreaterThanCutoff)) {
//			Serial << F("Time cutoff!") << endl;
//			valveOpen = false;
//			digitalWrite(valveControlPin, valveOpen);
//		}
		_500ms = millis();
	}

/*	if (millis() - _20ms > 20) {
		navigation(read_buttons());
		nav.poll();
		_20ms = millis();
	}

	if (millis() - _50ms > 50) {
		idleRefresh();
		_50ms = millis();
	}*/
}  // end of loop

void process_pulse() {
  pulseCount++;
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

		lcdi.buf[0] << F("") << milliLitres << F("ml");
		lcdi.buf[0] << F(" ") << pulseCount << F("");
		lcdi.buf[0] << F(" ") << valveOpen << F("");
		lcdi.buf[0] << F(" ") << litreLimit << F("l");

		lcdi.buf[1] << F("") << printTime() << F(" ");

		for (int i = 0; i < ROWS; i++) {
			while (lcdi.buf[i].current_length() < COLS) lcdi.buf[i] << ' ';
			lcd.setCursor(0, i);
			lcd.print(lcdi.buf[i]);
		}
	}
}
