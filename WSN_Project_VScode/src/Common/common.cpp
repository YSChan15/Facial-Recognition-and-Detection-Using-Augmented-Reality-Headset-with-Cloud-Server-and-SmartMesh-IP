#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <Math.h>
#include <algorithm>
#include <ctype.h> 
#include "common.h"
#include "RTCClass.h"

char *concat_strings(int num_strings, ...) {
   int total_length = 0;
	
	 //initialize the variable argument list for reading the strings
   va_list args;
   va_start(args, num_strings);
	
   // length of concatentated string
   for (int i = 0; i < num_strings; i++) {
      total_length += strlen(va_arg(args, char *));
   }
   va_end(args);
	 
	 //Allocate memmory for result string and initialize it
   char *result = (char *)malloc((total_length + 1) * sizeof(char));
   result[0] = '\0';
	 
   // Re-initialize the variable argument list for reading the strings
   va_start(args, num_strings);
	 
	 // Concatenate the strings
   for (int i = 0; i < num_strings; i++) {
      strcat(result, va_arg(args, char *));
   }
	 
   va_end(args);
   return result;
}

// Converts integer to ascii
char* int_to_ascii_base10(int number, int places){
	int buffer_size = (int)log10f((float)places);
	char* buffer = new char[buffer_size];
	
	int dataCount = 0;
	for(int i = places; i >=1; i/=10){
		char x = (number - (number % i))/i % 10 + 0x30;
		buffer[dataCount] = x;
		dataCount++;
	}
	
	return buffer;// Use sizeof to get character length
}

// Converts mac address to ascii
char* convertMacAddress(const char* mac) {
    // add one extra byte for the null terminator
    static char macAdress[24]; 
    int byt = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 1; j >= 0; j--) {
            byt = mac[i];
            byt = (byt >> 4 * j) & 0xF;
            if (byt < 10)
                macAdress[i * 3 + (1 - j)] = byt + 0x30;
            else
                macAdress[i * 3 + (1 - j)] = byt + 0x37;
        }
        if (i == 7) break;
        macAdress[2 + i * 3] = ':';
    }
    macAdress[23] = '\0'; // add the null terminator at the end of the string
    return macAdress;
}

bool setupRTC(const char *buf) {
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    sscanf(buf, "+CCLK: \"%02d/%02d/%02d,%02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);

    // Convert the hour to 12-hour format
    if (hour > 12) {
        hour -= 12;
    } else if (hour == 0) {
        hour = 12;
    }

    // Real-time clock setup
    timestamp stamp = {
        year + 2000,
        month,
        day,
        hour,
        minute,
        second
    };
    rtc.set_timestamp(stamp);

    return true;
}
