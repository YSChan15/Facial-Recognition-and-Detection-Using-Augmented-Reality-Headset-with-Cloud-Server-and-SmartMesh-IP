#ifndef __COMMON_H
#define __COMMON_H

#include <string>
#include <stdint.h>
#include "Freertos.h"
#include "queue.h"
#include "task.h"
#include "SERIALCOM.h"

/**
 * @brief This function concatenates multiple strings into a single string.
 * @param num_strings The number of strings to be concatenated.
 * @param ... The strings to be concatenated, passed as a variable number of arguments.
 * @return A dynamically allocated buffer containing the concatenated string.
 */
char *concat_strings(int num_strings, ...);

/**
 * @brief This function Converts MAC from binary to ASCII
 * i.e. 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB =  "01:23:45:67:89:AB"
 * @param mac The MAC address to be converted.
 * @return A dynamically allocated buffer containing the converted MAC address.
 */
char* convertMacAddress(const char* mac);

/**
 * @brief This function converts an integer into a string in base 10.
 * @param number The integer to be converted.
 * @param max_value The maximum value that the integer can have.
 * @return A dynamically allocated buffer containing the converted integer.
 */
char* int_to_ascii_base10(int number, int max_value);

/**
 * @brief This function sets up the Real-Time Clock.
 * @param buf A buffer containing the date and time information.
 * @return True if the RTC was set up successfully, false otherwise.
 */
bool setupRTC(const char *buf);
	

#endif
