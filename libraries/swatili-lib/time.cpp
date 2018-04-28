/*
 * time.cpp
 *
 *  Created on: 13 Sep 2017
 *      Author: d7rob
 */

// Interfacing with the DS1302 timekeeping chip.
//
// Copyright (c) 2009, Matt Sparks
// All rights reserved.
//
// http://quadpoint.org/projects/arduino-ds1302

#include "swatili.h"

DS1302 rtc(rtcCePin, rtcIoPin, rtcSclkPin);
Time t = rtc.time();
Time tReset = Time(2000, 1, 1, 4, 0, 0, Time::kSaturday);

//namespace {

void setup_rtc(Time tm) {
	// Initialize a new chip by turning off write protection and clearing the
	// clock halt flag. These methods needn't always be called. See the DS1302
	// datasheet for details.
	rtc.writeProtect(false);
	rtc.halt(false);

	// Make a new time object to set the date and time.
	// Sunday, September 22, 2013 at 01:38:50.
	// Time t(2018, 4, 28, 02, 44, 32, Time::kSaturday);

	// Set the time and date on the chip.
	rtc.time(tm);
}

/*String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sunday";
    case Time::kMonday: return "Monday";
    case Time::kTuesday: return "Tuesday";
    case Time::kWednesday: return "Wednesday";
    case Time::kThursday: return "Thursday";
    case Time::kFriday: return "Friday";
    case Time::kSaturday: return "Saturday";
  }
  return "(unknown day)";
}*/

const char * printTime() {
  // Get the current time and date from the chip.
  t = rtc.time();

  // Name the day of the week.
  // const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%02d%02d%02d %02d:%02d:%02d",
           /*day.c_str(),*/
           t.yr % 100, t.mon, t.date,
           t.hr, t.min, t.sec);

  // Print the formatted string to serial so we can see the time.
//  Serial.print(buf);
  return buf;
}

//}  // namespace
