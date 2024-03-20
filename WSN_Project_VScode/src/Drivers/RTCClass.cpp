#include "RTCClass.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

using namespace std;

// All years will be in reference to 2020
RTC_Class::RTC_Class(){
	RTC->MODE2.CTRLA.reg |= 0x9B0A;// Setup RTC and enable the counter
}
		
void RTC_Class::set_timestamp(timestamp times){
	this->time = times;
			
//	uint8_t year = times.year - REF_YEAR;
	RTC->MODE2.CLOCK .reg= (((times.year - REF_YEAR) << 26))|((times.month) << 22)|((times.day << 17))\
		|((times.hour << 12))|((times.minute << 6))|(times.second);
}
		
timestamp RTC_Class::get_timestamp(){
	timestamp tempTime = this->time;
	tempTime.year = time.year + REF_YEAR;// Readjust the year
			
	return tempTime;
}
		
char* RTC_Class::get_string_timestamp(){
	// Timestamp format mm/dd/yy hh:mm:ss => always in 24hr time
	char* buffer = new char[18];// Null character included
	
	timestamp tempTime = get_timestamp_from_reg();
	tempTime.year = tempTime.year + REF_YEAR;// Readjust the year
	
	char* stuff = int_to_ascii_base10(time.month, 10);
	memcpy(buffer, stuff, 2);
	delete stuff;
	
	buffer[2] = '-';
	stuff = int_to_ascii_base10(tempTime.day, 10);
	memcpy(buffer+3, stuff, 2);
	delete stuff;
	
	buffer[5] = '-';
	stuff = int_to_ascii_base10(tempTime.year, 1000);
	memcpy(buffer+6, stuff+2, 2);
	delete stuff;
	
	buffer[8] = ' ';
	stuff = int_to_ascii_base10(tempTime.hour, 10);
	memcpy(buffer+9, stuff, 2);
	delete stuff;
	
	buffer[11] = ':';
	stuff = int_to_ascii_base10(tempTime.minute, 10);
	memcpy(buffer+12, stuff, 2);
	delete stuff;
	
	buffer[14] = ':';
	stuff = int_to_ascii_base10(tempTime.second, 10);
	memcpy(buffer+15, stuff, 2);
	delete stuff;
	buffer[17] = '\0';
	
	return buffer;
}

timestamp RTC_Class::get_timestamp_from_reg(){
	timestamp currTime;
	
	currTime.year = RTC->MODE2.CLOCK.reg >> 26;
	currTime.month = (RTC->MODE2.CLOCK.reg >> 22) &0xF;
	currTime.day = (RTC->MODE2.CLOCK.reg >> 17) &0x1F;
	currTime.hour = (RTC->MODE2.CLOCK.reg >> 12) &0x1F;
	currTime.minute = (RTC->MODE2.CLOCK.reg >> 6) &0x3F;
	currTime.second = RTC->MODE2.CLOCK.reg & 0x3F;
	
	return currTime;
}
