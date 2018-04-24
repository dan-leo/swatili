/*
 * analog_comparator.cpp
 *
 *  Created on: 13 Sep 2017
 *      Author: d7rob
 */

#include "setup.h"

//extern unsigned long pulse_count;
//extern boolean triggered;

//void setup_analog_comparator() {
//	//	DIDR1 = bit (AIN0D);	// (Disable) Digital input buffer to save power. Always 0 when digitally read. Only analog in.
//	cbi(ADCSRA, ADEN);		// (Disable) ADC Enable
//////	sbi(PRR0, PRADC);
//	sbi(ADCSRB, ACME);		// (Enable) ACME: Analog Comparator Multiplexer Enable
//	ADMUX = 0;
//	ACSR =  bit (ACI)		// (Clear) Analog Comparator Interrupt Flag
//		| bit (ACIE)		// Analog Comparator Interrupt Enable
//	//	| bit(ACBG);		// Bandgap voltage (1.1V) connected
//		| bit (ACIS1);		// ACIS1, ACIS0: Analog Comparator Interrupt Mode Select (trigger on falling edge)
//
//}

//boolean service_trigger() {
//	static unsigned long _ms = 0;
//	static boolean _debouncing = false;
//
//
//
//
//
////	if (triggered)
////	{
////		_ms = millis();
//////		led_state = !led_state;
////		_debouncing = true;
////		digitalWrite(13, _debouncing);
////		triggered = false;
////	}
////
////	if (_debouncing && millis() - _ms > 5) {
////		_debouncing = false;
////		digitalWrite(13, _debouncing);
////		if (!bitRead(ACSR, ACO)) {
//////			Serial.println(pulse_count);
////			pulse_count++;
////			return true;
////		}
////	}
//	return false;
//}

//void analog_read(uint8_t analog_pin) {
//	sbi(ADCSRA, ADEN);
//////	cbi(PRR0, PRADC);
//	cbi(ADCSRB, ACME);
//	ACSR = 0;
//	Serial.println(analogRead(analog_pin)); // A5
//	setup_analog_comparator();
//	delay(1);
//}
