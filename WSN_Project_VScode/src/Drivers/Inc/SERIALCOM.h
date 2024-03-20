#ifndef __SERCOM_H
#define __SERCOM_H

#include "sam.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "Freertos.h"
#include "semphr.h"
#include <string>

#define UART_BUFFER_SIZE 	256
#define SERCOM_NUM			6

class UART;

typedef struct{
	UART *class_inst;
	Sercom *SERCOM_inst;
}UART_node_t;

class UART{
	public:
		UART(Sercom *port, int baudrate, TickType_t semDelay = portMAX_DELAY);
		int _printf(const char *format, ...);
		void send_array(uint8_t *data, uint8_t length);
		static BaseType_t takeSem();
		static BaseType_t giveSem();
		inline bool isInstance(UART *inst);
		static UART& getInst(Sercom *sercom_used);

	private:
		SercomUsart* UART_port;// UART port to use
		uint8_t dma_channel_id;
		TickType_t semDelayTicks;
		
		static SemaphoreHandle_t bluetoothIsUsed;
		static UART_node_t instances[SERCOM_NUM + 1];
};

#endif
