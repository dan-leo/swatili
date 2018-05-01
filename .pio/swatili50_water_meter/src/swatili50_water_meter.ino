/*
 * main.ino
 *
 *  Created on: 22 Apr 2018
 *      Author: d7rob
 */


#include <Arduino.h> /* Arduino code base for compiler */

#include "swatili.h" /* Look inside the swatili library in libraries folder for more code */

float k = 1.0; /* A constant to calibrate the accuracy of the pulse meter */
unsigned long pulseCount = 0; /* Counts actual pulses */
unsigned long milliLitres = 0; /* millilitres calculated from pulse count */
float litreLimit = 0.5; /* Litre limit before the system shuts the solenoid valve */

volatile byte led_state = LOW; /* a variable which helps the LED to blink per pulse */

/* Flag which is serviced in the main loop, especially considering that
 * the pulse interrupts happen faster than the main loop, so it only has to do the
 * float calculations to millilitres when necessary */
boolean pulseProcessedFlag = true;

boolean menuIdling = false; /* boolean which knows if the menu is on the home / idle screen */
boolean valveOpen = false; /* boolean which stores the state of the solenoid valve */

extern Time t; /* System time */
extern Time tReset; /* Daily pulse reset time. Only hr, min, sec is stored. */

/* Press-able buttons to control the menu, reset the pulse counter and control
 * the solenoid valve. `true` means that each pin is pulled up by a resistor.
 * That means that the pins are not floating. Floating means that ambient electricity
 * from the mains can affect each pin. The // x numbers are numbers that can be used
 * if one wishes to flip the order of buttons upside-down. */
Button btBack (buttonPins[0], true); // 7
Button btDown (buttonPins[1], true); // 6
Button btUp   (buttonPins[2], true); // 5
Button btEnter(buttonPins[3], true); // 4
Button btReset(buttonPins[4], true); // 3
Button btValve(buttonPins[5], true); // 2

void process_pulse();                           /* function prototype */

/*##################################### menu code #####################################*/
/**/

enum buttons_in read_buttons();                 /* function prototype */
void navigation(enum buttons_in button);        /* function prototype */
void idleRefresh();                             /* function prototype */
result resetTime(eventMask e, prompt &item);    /* function prototype */
result resetTime_alert(menuOut& o,idleEvent e); /* function prototype */

LiquidCrystal_I2C lcd(0x0); /* Connect via i2c, default address #0x0 (A0-A2 not jumpered) */
LCD_idler lcdi;             /* LCD idler class handles text updating on home / idle screen. */

/* Serial input for ArduinoMenu library */
serialIn serial(Serial);
menuIn* inputsList[]={&serial}; // more inputs can be added here.
chainStream<1> in(inputsList); // 1 is the current number of inputs

result idle(menuOut& o,idleEvent e)
	/* When the menu is idling, it runs this function as a sleep task
	 * */ {
	switch(e) {
	    case idleStart: menuIdling = true; break;
	    case idling: /*o.setCursor(0, 0); o.println(F("Water Meter v0.1"));*/ break;
	    case idleEnd: menuIdling = false; break;
	}
	return proceed;
}


result updateTime(eventMask e, navNode& nav, prompt &item)
	/* ArduinoMenu hook to update the time upon exit event
	 * */ {
	Serial << F("New time set!") << endl;
	setup_rtc(t);
	return proceed;
}

/* Update system time */
MENU(timeSubmenu,"Time setup",doNothing,anyEvent,noStyle
  ,FIELD(t.yr,"Year","",2000,2099,10,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.mon,"Month","",1,12,1,0,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.date,"Date","",1,31,10,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.hr,"Hours","",0,23,5,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.min,"Minutes","",0,59,10,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(t.sec,"Seconds","",0,59,10,1,updateTime,exitEvent,wrapStyle)
  ,EXIT("<Back")
);

/* Update the daily time to reset pulse and litre count */
MENU(resetLitresSubmenu,"T reset litres",doNothing,anyEvent,noStyle
  ,FIELD(tReset.hr,"Hour","",0,23,5,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(tReset.min,"Minute","",0,59,10,1,updateTime,exitEvent,wrapStyle)
  ,FIELD(tReset.sec,"Second","",0,59,10,1,updateTime,exitEvent,wrapStyle)
  ,EXIT("<Back")
);

/* Reset system time to a default time. Useful if 3V battery pulled out */
MENU(resetTimeSubmenu,"Reset Time",doNothing,anyEvent,noStyle
  ,OP("Yes",resetTime,enterEvent)
  ,OP("No",doNothing,enterEvent)
  ,EXIT("<Back")
);

/* Main menu of water meter */
MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,FIELD(pulseCount,"Pulses","",0,0,0,0,doNothing,updateEvent,wrapStyle)
  ,FIELD(milliLitres,"Volume","ml",0,0,0,0,doNothing,updateEvent,wrapStyle)
  ,FIELD(litreLimit,"Limit","l",0.0,50.0,1,0.1,doNothing,updateEvent,wrapStyle)
  ,FIELD(k,"k constant","",0,3,0.1,0.01,doNothing,updateEvent,wrapStyle)
  ,SUBMENU(timeSubmenu)
  ,SUBMENU(resetLitresSubmenu)
  ,SUBMENU(resetTimeSubmenu)
  ,EXIT("<Back")
);

/* ArduinoMenu code to simultaneously print on Serial and LCD at the same time. */
#define MAX_DEPTH 4
MENU_OUTPUTS(out, MAX_DEPTH
  ,LCD_OUT(lcd,{0,0,16,2})
  ,SERIAL_OUT(Serial)
  ,NONE
);
NAVROOT(nav,mainMenu,MAX_DEPTH,in,out); //the navigation root object

result resetTime(eventMask e, prompt &item)
	/* Reset time to default. ArduinoMenu hook.
	 * */ {
	t = Time(2018, 4, 28, 04, 48, 00, Time::kSaturday);
	setup_rtc(t);
	nav.idleOn(resetTime_alert);
	return proceed;
}

result resetTime_alert(menuOut& o,idleEvent e)
	/* Confirm time reset
	 * */ {
	if (e==idling) {
		o.setCursor(0,0);
		o.print("Time reset.");
		o.setCursor(0,1); o.print("[enter] to cont.");
	}
	return proceed;
}

/**/
/*##################################### end menu code #####################################*/

void setup ()
{
	/* Serial port can be connected to with an FTDI or Serial-USB converter at 115200 baud rate */
	Serial.begin(115200);

	lcd.begin(16, 2); /* initialize 16x2 LCD display */
	lcd.setBacklight(HIGH); /* set backlight on */
	lcd.setCursor(0, 0); lcd.print(F("Water Meter v1.0"));
	lcd.setCursor(0, 1); lcd.print(F(" Andre van Dyk  "));
	delay(500);

	pinMode(LED_BUILTIN, OUTPUT); /* set on-board LED pin for output */
	pinMode(valveControlPin, OUTPUT); /* set valve control pin for output */
	pinMode(pulseInterruptPin, INPUT_PULLUP); /* pullup resistor for pulse interupt pin */

	/* attach an interrupt on falling edge of pulse meter for process_pulse() function */
	attachInterrupt(digitalPinToInterrupt(pulseInterruptPin), process_pulse, FALLING);

	nav.idleTask=idle; /* point a function to be used when menu is suspended */
	mainMenu[0].enabled = disabledStatus; /* disable selection of edit variable, */
	mainMenu[1].enabled = disabledStatus; /* which is useful just to view */
	nav.showTitle = false;
	nav.doNav(escCmd); nav.poll(); /* start on home / idle screen */

}

void loop ()
{
	/* schedule variables for sub loops */
	static unsigned long _10ms = millis(),
                         _100ms = millis(),
                         _500ms = millis();

	digitalWrite(LED_BUILTIN, led_state); /* blink for every pulse */

	/* process interrupt pulses */
	if (pulseProcessedFlag) {
		milliLitres = pulseCount * 2.25 * k;
		pulseProcessedFlag = false;
	}

	/* check if water volume greater than limit allowed */
	if (milliLitres * 1.0 > litreLimit * 1000) {
		valveOpen = false;
		digitalWrite(valveControlPin, valveOpen);
	}

	/* check buttons */
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

	/* refresh idle screen */
	if (millis() - _100ms > 100) {
		idleRefresh();
		_100ms = millis();
	}

	/* check system time for reset time */
	if (millis() - _500ms > 500) {
		if (t.hr == tReset.hr &&
				t.min == tReset.min &&
				t.sec == tReset.sec) {
			pulseCount = 0;
			milliLitres = 0;
		}
		_500ms = millis();
	}

}

void process_pulse()
	/* increment pulse counter for every falling edge pulse
	 * */ {
	pulseCount++;
	led_state = !led_state;
	pulseProcessedFlag = true;
}

enum buttons_in read_buttons()
	/* read navigation buttons
	 * */ {
	if      (btEnter.check() == LOW) return bt_enter;
	else if (btBack.check()  == LOW) return bt_back;
	else if (btUp.check()    == LOW) return bt_incr;
	else if (btDown.check()  == LOW) return bt_decr;
	return bt_none;
}

void navigation(enum buttons_in button)
	/* navigate menu with read buttons
	 * */ {
	if      (button == bt_enter) nav.doNav(enterCmd);
	else if (button == bt_back)  nav.doNav(escCmd  );
	else if (button == bt_incr)  nav.doNav(upCmd   );
	else if (button == bt_decr)  nav.doNav(downCmd );
}

void idleRefresh()
	/* if menu idling, update the text and variables
	 * using buffers for each line. Avoids using lcd.clear()
	 * which creates a perceptible moment of nothing on display.
	 * */ {

	if (menuIdling) {
		for (int i = 0; i < ROWS; i++) {
			lcdi.buf[i].flush();
		}

		/* populate first line with data */
		lcdi.buf[0] << F("") << milliLitres << F("ml");
		lcdi.buf[0] << F(" ") << pulseCount << F("");
		lcdi.buf[0] << F(" ") << valveOpen << F("");
		lcdi.buf[0] << F(" ") << litreLimit << F("l");

		/* populate second line with time */
		lcdi.buf[1] << F("") << printTime() << F(" ");

		/* print buffered lines to LCD */
		for (int i = 0; i < ROWS; i++) {
			while (lcdi.buf[i].current_length() < COLS) lcdi.buf[i] << ' ';
			lcd.setCursor(0, i);
			lcd.print(lcdi.buf[i]);
		}
	}
}
