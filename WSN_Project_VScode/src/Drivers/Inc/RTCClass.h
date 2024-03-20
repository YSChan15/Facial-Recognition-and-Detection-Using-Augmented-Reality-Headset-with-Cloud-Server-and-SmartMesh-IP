#ifndef __RTC_H
#define __RTC_H

#include "saml21g18b.h"
#include <stdint.h>

#define REF_YEAR 2020

// Timestamp struct
typedef struct{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
}timestamp;

class RTC_Class{
	public:
		RTC_Class();
		void set_timestamp(timestamp);
		timestamp get_timestamp();
		char* get_string_timestamp();
	
	private:
		timestamp time;
	
		timestamp get_timestamp_from_reg();
};

extern RTC_Class rtc;

#endif
