/*
 * \file bluetooth.cpp
 * \author Arteom Katkov
 * \brief State Machine Handler for Bluetooth Data Comming In
 * \details Reads characters from the UART interrupt handler and gets the requested data or
              sets the proper parameters
*/

#include <stdint.h>
#include "Freertos.h"
#include "queue.h"
#include "task.h"
#include "SERIALCOM.h"
#include "API.h"
#include "bluetooth.h"
#include "sm_parse.h"

// RTOS Semaphores and queues
static QueueHandle_t bluetoothData = xQueueCreate(48, sizeof(uint8_t));

// Bluetooth states
typedef enum{
	STATE_RUNNING,
	STATE_NETID,
	STATE_JKEY,
	STATE_MOTE_LIST,
	STATE_GET_NET_INFO,
	STATE_MOTE_INFO,
	STATE_MGR_CONNECTED,
	STATE_CLEAR_STATS,
	STATE_CMD_ERROR,
	
	STATE_AMOUNT// Must be last
}bt_states_t;

// Error States
typedef enum{
	ERROR_NONE,
	ERROR_NO_MOTES_FOUND,
	ERROR_SEM_TAKE,
	ERROR_UNDEF,
	
	ERROR_AMT
}error_states_t;

// Private event handler function definitions
static void runningCallback(UART &bluetoothInstance);
static void setNetID_Handler(UART &bluetoothInstance);
static void setJKeyHandler(UART &bluetoothInstance);
static void getMoteListHandler(UART &bluetoothInstance);
static void getNetInfoHandler(UART &bluetoothInstance);
static void getMoteInfoHandler(UART &bluetoothInstance);
static void isMgrConnectedHandler(UART &bluetoothInstance);
static void clearStatsHandler(UART &bluetoothInstance);
static void errorHandler(UART &bluetoothInstance);

// Private variable definitions
// Structure for event holding event handlers
struct{
	bt_states_t state; // Optional can be removed
	void (*handler)(UART &bluetoothInstance);
}static handler_table[STATE_AMOUNT] = {
	[STATE_RUNNING]			= {STATE_RUNNING, 		&runningCallback},
	[STATE_NETID]			= {STATE_NETID,			&setNetID_Handler},
	[STATE_JKEY]			= {STATE_JKEY,			&setJKeyHandler},
	[STATE_MOTE_LIST]		= {STATE_MOTE_LIST,		&getMoteListHandler},
	[STATE_GET_NET_INFO]	= {STATE_GET_NET_INFO,	&getNetInfoHandler},
	[STATE_MOTE_INFO]		= {STATE_MOTE_INFO,		&getMoteInfoHandler},
	[STATE_MGR_CONNECTED]	= {STATE_MGR_CONNECTED,	&isMgrConnectedHandler},
	[STATE_CLEAR_STATS]		= {STATE_CLEAR_STATS,	&clearStatsHandler},
	[STATE_CMD_ERROR]		= {STATE_CMD_ERROR,		&errorHandler}
};
static bt_states_t curr_state = STATE_RUNNING;
static error_states_t error_state = ERROR_NONE;

// This task will parse any data recived thorugh bluetooth
void bluetoothParse(void* unused)
{
	UART bluetoothInstance = UART::getInst(SERCOM0);

	while(1)
	{
		handler_table[curr_state].handler(bluetoothInstance);// Execute the correct callback function
	}

	vTaskDelete(nullptr);// Should not reach this point
}

//************************ EVENT HANDLERS **************************//
static void runningCallback(UART &bluetoothInstance)
{
	uint8_t recieved_data;
	
	xQueueReceive(bluetoothData, &recieved_data, portMAX_DELAY);// Wait for data to come in
	switch(recieved_data){// Main state machine for bluetooth data
    case 'A':// Network ID recieved
    case 'B':// Setup common join key
    case 'C':// Get the mote list
      curr_state = (bt_states_t)(recieved_data - 'A' + 1);
      break;
      
    case 'G':// Get the current network information
      curr_state = STATE_GET_NET_INFO;
      break;

    case 'I':// Get mote information
      curr_state = STATE_MOTE_INFO;
      break;
      
    case 'E':// Has a connection been established with the network manager
      curr_state = STATE_MGR_CONNECTED;
      break;
      
    case 'H':// Clear statistics command
      curr_state = STATE_CLEAR_STATS;
      break;
      
    default:// TODO: add improved default handling
      curr_state = STATE_CMD_ERROR;
      error_state = ERROR_UNDEF;
      break;
  }
}

static void setNetID_Handler(UART &bluetoothInstance)
{
	uint8_t netid[2];
	
	xQueueReceive(bluetoothData, &netid[0], portMAX_DELAY);
	xQueueReceive(bluetoothData, &netid[1], portMAX_DELAY);
	
	api.setNetworkConfig(netid);// Setup the network config
	
	curr_state = STATE_RUNNING;
}

static void setJKeyHandler(UART &bluetoothInstance)
{
	uint8_t jkey[16];
	for(int i = 0;i < 16;i++)
		xQueueReceive(bluetoothData, &jkey[i], portMAX_DELAY);// Get the joinkey
	
	api.setJoinKey(jkey);
	
	curr_state = STATE_RUNNING;
}

static void getMoteListHandler(UART &bluetoothInstance)
{
	uint16_t motes;
	api.getNetworkInfo();
	
	if(get_packet_status(GET_NETWORK_INFO) == pdFALSE)
	{
		api.mgr_init();	
		bluetoothInstance._printf("X");
	}
	
	motes = (smartmeshData[6] << 8)|smartmeshData[7];
	if(motes == 0)// No motes are currently on the network
	{
		error_state = ERROR_NO_MOTES_FOUND;
		errorHandler(bluetoothInstance); // TODO: Implement better error handling
		return;
	}
	
	bluetoothInstance._printf("C");
	for(int i = 1;i <= motes;i++){// Step 2 => get all the mote mac addresses
		api.getMoteConfigFromMoteId(i+1);
		if(get_packet_status(GET_MOTE_CFG_BY_ID) == pdFALSE)
		{
			api.mgr_init();	
			bluetoothInstance._printf("X");
		}
		bluetoothInstance._printf("D");
		bluetoothInstance.send_array(smartmeshData+6, 8);
	}
	bluetoothInstance._printf("E");
	
	curr_state = STATE_RUNNING;
}

static void getNetInfoHandler(UART &bluetoothInstance)
{
	network_config config;
	network_info info;
	
	api.getNetworkConfig();

	if(get_packet_status(GET_NET_CONFIG) == pdFALSE)
	{
		api.mgr_init();	
		bluetoothInstance._printf("X");
	}	
	
	api.parseNetworkConfig(&config, smartmeshData);
				
	api.getNetworkInfo();
				
	if(get_packet_status(GET_NETWORK_INFO) == pdFALSE)
	{
		api.mgr_init();	
		bluetoothInstance._printf("X");
	}
			
	api.parseNetworkInfo(&info, smartmeshData);
			
	bluetoothInstance._printf("J");// Send network manager configuration
	bluetoothInstance.send_array((uint8_t*)&config.networkId, 2);// 1. Send the network ID(2 bytes)
	bluetoothInstance.send_array((uint8_t*)&config.apTxPower, 1);// 2. Send the TX power of the manager(1 byte)	
			
	bluetoothInstance.send_array((uint8_t*)&info.num_motes, 2);// 3. Number of motes currently connected(2 bytes)
	bluetoothInstance.send_array((uint8_t*)&info.ipv6AddrHigh, 16);// 4. Send IPV6 address(16 bytes)
	
	curr_state = STATE_RUNNING;
}

static void getMoteInfoHandler(UART &bluetoothInstance)
{
	uint8_t mac_addr1[8];
	for(int i = 0;i < 8;i++)
		xQueueReceive(bluetoothData, &mac_addr1[i], portMAX_DELAY);// Get the mac address
	api.getMoteInfo(mac_addr1);
	if(get_packet_status(GET_MOTE_INFO) == pdFALSE)
	{
		api.mgr_init();	
		bluetoothInstance._printf("X");
	}

	curr_state = STATE_RUNNING;
}

static void isMgrConnectedHandler(UART &bluetoothInstance)
{	
	if(mgr_connection_status() == true)
		bluetoothInstance._printf("K1");
	else
		bluetoothInstance._printf("K0");
	
	curr_state = STATE_RUNNING;
}

static void clearStatsHandler(UART &bluetoothInstance)
{
	api.clearStatistics();
	vTaskDelay(100);	
	
	bluetoothInstance._printf("M");
	
	curr_state = STATE_RUNNING;
}

static void errorHandler(UART &bluetoothInstance)
{
	if(error_state == ERROR_UNDEF)
		bluetoothInstance._printf("X");
	
	curr_state = STATE_RUNNING;
	error_state = ERROR_NONE;
}

//************************INTERRUPT HANDLERS**************************//
extern "C"{
	void SERCOM0_Handler(void){// Bluetooth handler
		uint8_t data = SERCOM0->USART.DATA.reg;

		xQueueSendFromISR(bluetoothData, &data, NULL);// Send data to the bluetooth queue
		
		NVIC_ClearPendingIRQ(SERCOM0_IRQn);
	}
};