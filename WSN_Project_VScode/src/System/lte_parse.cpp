#include <lte_parse.h>
#include "bluetooth.h"
#include <string.h>
#include <algorithm>
#include <ctype.h> 
#include <stdlib.h>
#include <stdio.h>
#include "RTCClass.h"
#include "DMA.h"
#include "common.h"
#include "semphr.h"
#include <array>

// Queue to hold data recieved from the LTE module (upto LTE_BUFFER_SIZE bytes)
static xQueueHandle lte_queue = xQueueCreate(512, sizeof(uint8_t)); 

// api.sendData variables
				// MAC Address: All (macAddr) = 0xFFFFFFFFFFFFFFFF
				// Priority: Default 		  = 0
				// Manager port (src)         = 0xF0B8
				// Mote port (dst) 			  = 0xF0B8
				// Server data (data) 		  = response_buffer
				// Data length (length) 	  = responseLength 
				// sendData(uint64_t macAddr, uint8_t priority, uint16_t src, uint16_t dst, uint8_t *data, uint8_t length)
const static uint64_t ALL_MOTES =  0xFFFFFFFFFFFFFFFF;
const static uint8_t SRC_PORT[2] = {0xF0, 0xB8}; // Source SmartMesh Port
const static uint8_t DST_PORT[2] = {0xF0, 0xB8}; // Destination SmartMesh Port


extern SemaphoreHandle_t smartmesh_connected; 	// SmartMesh Connection Semaphore 
static char lteBuffer[LTE_BUFFER_SIZE]; 		// Buffer used for DMA transfers
static char response_buffer[LTE_BUFFER_SIZE]; 	// UART RX Buffer	
static uint8_t responseLength;					// Length of RX Response
static uint8_t byte_data = 0;				    // Variable to hold RX byte


// LTE module state variables
typedef enum{
	STATE_INIT,
	STATE_RECIEVING,
	STATE_PARSING,
}states_t;

static states_t curr_state = STATE_INIT; // Current state of LTE state machine

// static LTE_Parse parse;
// static std::array<char, LTE_BUFFER_SIZE> response_buffer;

// Task to initialize module and connect to network
void beginLTE(void* unused)
{
	lte.init(); 			// Initialize LTE module
	lte.connectToServer();  // Connect module to hologram network and TCP server
	vTaskDelete(NULL); 		// Delete task since it is not needed anymore
}

// Function to parse the LTE module responses
void parseResponse(char* response, uint8_t* length)
{
	// Copy response over to the local string 'response' 
	DMAC_Driver::DMAC_Send_Arr((uint8_t*)lteBuffer, (uint16_t)*length + 2, 
			(uint8_t)0, (uint32_t)response);


	std::string resp_string(response);
	lte.parse(resp_string);				// Parse Response

}

// This task will parse any data recived thorugh LTE
void lte_parse_task(void* unused){
	uint8_t data;

	while(1)
	{
		switch(curr_state)
		{
			case STATE_INIT:
				// Create a task to initialize the LTE module and connect it to the network
				if(xSemaphoreTake(smartmesh_connected, portMAX_DELAY) == pdTRUE)
				{
					xTaskCreate(beginLTE, "Begin LTE Setup", 64, NULL, 3, NULL);
					curr_state = STATE_RECIEVING;
				}
				break;

			case STATE_RECIEVING:
				xQueueReceive(lte_queue, &data, portMAX_DELAY); // Wait for data to be received 
				response_buffer[responseLength] = data; // Add data to the response buffer
				responseLength++;

				// Check if end of line character is received
				if(data == '\n')
				{
					response_buffer[responseLength] = '\0';
					curr_state = STATE_PARSING;
				}
				break;

			case STATE_PARSING:
				// If response is a clock response, set the real-time clock with the time received
				if (lte.isClockResponse(response_buffer)){ 
					setupRTC(response_buffer);
					
				}
				// Otherwise parse the response
				else {

					DMAC_Driver::DMAC_Send_Arr((uint8_t*)response_buffer, (uint16_t)responseLength + 2, 
						(uint8_t)0, (uint32_t)lteBuffer, false);
					parseResponse(response_buffer, &responseLength);
				}

				// If server is connected, send server response
				if(lte.serverConnected && (responseLength > 2))
				{
					api.sendData(ALL_MOTES,PACKET_PRIORITY_HIGH,SRC_PORT,DST_PORT,(uint8_t*)response_buffer,responseLength + 1);
				}

				responseLength = 0; // Reset response length
				curr_state = STATE_RECIEVING;
				break;
		}
	}
}

// LTE Module interrupt handler
extern "C"{
	void SERCOM2_Handler(void){
		
		// Add the data received from the LTE module to the queue while buffer is not empty
		/*while(SERCOM2->USART.INTFLAG.bit.RXC){ 
			data_buffer[data_index++] = SERCOM2->USART.DATA.reg;
			xQueueSendFromISR(lte_queue, &data_buffer[data_index - 1], nullptr);
		}
		data_index = 0;
		NVIC_ClearPendingIRQ(SERCOM2_IRQn);*/

		byte_data = SERCOM2->USART.DATA.reg;
		xQueueSendFromISR(lte_queue, &byte_data, nullptr);
		NVIC_ClearPendingIRQ(SERCOM2_IRQn);
	}
};

/*
// Constructor
LTE_Parse::LTE_Parse()
{
	// assert(s_instance == nullptr)
	s_instance = this;
}

// Runs the parsing task from global instance
void LTE_Parse_Task(void *unsued)
{
	LTE_Parse::GetInstance()->Task();
}
*/