#ifndef _LTE_H
#define _LTE_H

#include <string>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "SERIALCOM.h"
#include "bluetooth.h"
#include "semphr.h"

// SIM7000 module class
// Must be instantiated with uart reference to specify uart port
class LTE {
public:
  LTE(UART *uart);  
  bool init();
  bool connectToNetwork();
  bool connectToServer();
  bool disconnect();
  bool isConnected();
  bool sendCommand(const char* cmd);
  bool parse(std::string &response); // changed
  bool isClockResponse(const char* response);
  bool isValidIP(std::string &s);
	bool sendHTTPMessage(const char* msg, const char* url);
  bool send_to_firebase(uint8_t *data, uint8_t* mac);
  void checkDebugInfo(); //only used for debugging
  static bool serverConnected; 						// Server Connection Flag

  // These methods return pdTRue/pdFalse for pass/fail
  // and can be used inside/outside interrupts
  BaseType_t giveSem(bool fromISR); // Give LTE_used semaphore
  BaseType_t takeSem(bool fromISR); 

private:
  UART *_usart; // The uart port to send data to
  SemaphoreHandle_t sendingData;
  SemaphoreHandle_t LTE_used;
  
};


#endif

