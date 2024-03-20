/*
 * \file sm_parse.cpp
 * \author Arteom Katkov
 * \brief This file implements the smartmesh IP API packet parsing methods
*/

#include "sam.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "sm_parse.h"
#include "bluetooth.h"
#include "DMA.h"
#include "semphr.h"
#include "timers.h"
#include "common.h"
#include "lte_parse.h"
#include "send_task.h"

// Private Function Defines
static void parseSmartmeshData(void);
static bool reconstructPacket(uint8_t packet[]);

// Private Variable Defines
static QueueHandle_t apiPacketBuffer = xQueueCreate(255, sizeof(uint8_t));
static SemaphoreHandle_t found_packet = xSemaphoreCreateBinary();

static volatile states curr_state = STATE_INIT;

static volatile uint8_t length = 0;
static volatile uint8_t buffer[133];

struct S{uint8_t mac[8];};
static S mac_addresses[10];
static uint8_t macsAdded = 0;
static uint8_t slow = 0;

static bool connectedToManager = false;
static packet_type_t packet_to_get;

extern SemaphoreHandle_t smartmesh_connected;

const char *connect_response = "Connect";

/*
 * \fn smartmesh_parse_task
 * \brief Smartmesh Data Parsing Task
 * \param unused Unused Pointer needed to instatiate the task with FreeRTOS
 * \returns None
*/
void smartmesh_parse_task(void* unused){
	vTaskDelay(2500);
	api.mgr_init();// Initialize connection with the network manager

	uint8_t data;
	configASSERT(apiPacketBuffer != 0);
	
	while(1){
		if(curr_state != STATE_END_FOUND && curr_state != STATE_INIT)
			xQueueReceive(apiPacketBuffer, &data, portMAX_DELAY);

		switch (curr_state)
		{
			case STATE_INIT:
				api.mgr_init();

				curr_state = STATE_IDLE;
				break;

			case STATE_IDLE:
				if(data == API_FLAG)
				{
					length = 0;
					buffer[length] = data;

					curr_state = STATE_START_FOUND;
				}
				break;

			case STATE_START_FOUND:
				buffer[length+1] = data;
				length++;

				if(data == API_FLAG && length > 4)
				{
					curr_state = STATE_END_FOUND;
				}
				else if(data == API_FLAG && length <= 4)
				{
					length = 0;
				}

				break;

			case STATE_END_FOUND:// \todo Validate Recieved Packets
				curr_state = STATE_IDLE;

				DMAC_Driver::DMAC_Send_Arr((uint8_t*)buffer, (uint16_t)length + 2, 
					(uint8_t)0, (uint32_t)smartmeshData, true);

				// Alert requesting task that the packet has been recieved
				if(packet_to_get != 0 && packet_to_get == buffer[2])
				{
					packet_to_get = 0;
					xSemaphoreGive(found_packet);
				}

				reconstructPacket((uint8_t *)buffer);

				parseSmartmeshData();

				break;

			case STATE_ERROR:
				configASSERT(0);
				break;
			
			default:
				curr_state = STATE_ERROR;
				break;
		}
	}
}

/*
 * \fn parseSmartmeshData
 * \brief Parses Smartmesh IP Packets
 * \param Void
 * \returns Void
*/
static void parseSmartmeshData(void){
	UART bluetooth = UART::getInst(SERCOM0);
	
	uint8_t packetType = buffer[2];
	switch(packetType){
		case MGR_HELLO_RESPONSE:
		{
			api.subscribe(0xFFFFFFFF,0xFFFFFFFF);// Subscribe to notifications
			connectedToManager = true;// Set that the network manager is connected to the MCU
			break;
		}
		
		case MGR_HELLO:// No communication was setup
		{
			connectedToManager = false;// No connection has been established
			api.mgr_init();// Send hello packet
			break;
		}
		
		case NOTIF:// Notification packet recieved
		{
			//configASSERT(0); //debug

			if(buffer[5] != 0x4)
				break;

			data_notif notif;
			api.parseDataNotification(&notif, (uint8_t*)buffer);
			
			// Check if mac address exists
			uint8_t *temp = (uint8_t*)&notif.macAddr;
			int field_num = 1;
			bool mac_address_found = false;
			for(int i = 0;i < 2;i++){
				for(int j = 0;j < 8;j++){// Compare each byte
					if(mac_addresses[i].mac[j] != temp[j])// If not equal break
						break;
					else if(mac_addresses[i].mac[j] == temp[j] && j == 7)// Mac Address was found in list
						mac_address_found = true;
				}
				// Check if mac address was found
				if(mac_address_found){
					break;
				}
				field_num++;
			}

			// Construct a data packet to send to LTE module
			packet_t my_packet = {
				.data = notif.data,
				.mac_address = notif.macAddr
			};	

			// Allow connection to LTE if SmartMesh is connected.
			const char *sm_response = (char*)my_packet.data; // packet data in string format
			if((strstr(sm_response, connect_response) != NULL))
			{
				//configASSERT(0); Debug
				configASSERT(xSemaphoreGive(smartmesh_connected) == pdTRUE);// give semaphore to connect to LTE
			}
			
			
			// Send packet to firebase
			if(!mac_address_found){// No such mac address exists
				for(int i = 0;i < 8;i++) {
					mac_addresses[macsAdded].mac[i] = temp[i];
				}
					// Add packet to queue so it can be sent by the send_task
					enqueue_packet(my_packet);
					macsAdded++;
			}else{
				// Add packet to queue so it can be sent by the send_task
				enqueue_packet(my_packet);
			}
				
			slow++;
			
			bluetooth._printf("I");// Send data to C#
			bluetooth.send_array((uint8_t*)&notif.macAddr, 8);// Send the mac address
			bluetooth.send_array(notif.data, 8);// Send the data that was recieved

			break;
		}
		
		case SET_NTWK_CONFIG:// Network configuration was setup
		{
			bluetooth._printf("A");
			break;
		}
		
		case SET_COMMON_JKEY:// Network join key was set
		{
			bluetooth._printf("B");
			api.resetManager();
			break;
		}

		case GET_MOTE_INFO:
		{
			bluetooth._printf("F");// Mote information command
			bluetooth.send_array((uint8_t*)buffer + 6, 8);// Send mac address
			bluetooth.send_array((uint8_t*)buffer + 29, 12);
			break;
		}
		
		case RESET_CMD:
		{
			api.mgr_init();// Send hello packet
			break;
		}
	}
}

/*
 * \fn 		reconstructPacket
 * \brief 	Reconstruct packet data and remove control escape characters
 * \param 	packet Array with the packet data within
 * \return	True is packet reconstrcuted successfully
 * \retval  True/False
*/
static bool reconstructPacket(uint8_t packet[])
{
	for(int i = 1;packet[i] != 0x7E && i < 134;i++)
	{
		if(packet[i] == CONTROL_ESCAPE)// Next byte's 5th bit bmust be inverted
		{
			// Move data over one bit and delete control escape byte
			packet[i + 1] = GET_BYTE(packet[i + 1]);// Get proper data

			for(int j = i + 1; packet[j] != 0x7E && j < 134;j++)
			{
				packet[j - 1] = packet[j];// Shift data over
			}
		}
	}

	return true;
}

bool mgr_connection_status(void)
{
	return connectedToManager;
}

BaseType_t get_packet_status(packet_type_t packet)
{
	BaseType_t retval = pdTRUE;

	packet_to_get = packet;
	retval = xSemaphoreTake(found_packet, 2000);

	return retval;
}

extern "C"{
	void SERCOM1_Handler(void){
		uint8_t data = SERCOM1->USART.DATA.reg;

		configASSERT(xQueueSendFromISR(apiPacketBuffer, &data, nullptr) == pdTRUE);
		
		NVIC_ClearPendingIRQ(SERCOM1_IRQn);// Clear the interrupt
	}
};
