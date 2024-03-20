#include "API.h"
#include "Freertos.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "bluetooth.h"
#include "lte_parse.h"
#include "RTCClass.h"
#include "common.h"
#include "LTE.h"
#include "system_setup.h"
#include "saml21.h"
#include "DMA.h"
#include "sm_parse.h"
#include "send_task.h"

// Global variables
uint8_t smartmeshData[130];
RTC_Class rtc = RTC_Class();
Smartmesh_API api = Smartmesh_API((UART*)nullptr);
LTE lte = LTE((UART*)nullptr);
SemaphoreHandle_t smartmesh_connected;

// Main entry point into the program
 int main(){

	setup_system(); // Setup all peripherals

	// Class instantiation/initializations
	UART api_usart = UART(SERCOM1, 115200);
	UART bluetooth = UART(SERCOM0, 115200);
	UART lte_usart = UART(SERCOM2, 115200);
	api = Smartmesh_API(&api_usart);
	lte = LTE(&lte_usart);
	bluetooth._printf("Y"); // Notify GUI/App that system is running
	
	smartmesh_connected = xSemaphoreCreateBinary(); // Semaphore used for signaling successful SmartMesh connection

	// Create the tasks needed for the project
	xTaskCreate(lte_parse_task, "Parse LTE Data", 1024, NULL, 1, NULL); 
	// xTaskCreate(LTE_Parse_Task, "Send LTE Data", 384, NULL, 3, NULL);
	xTaskCreate(bluetoothParse, "Parse Bluetooth Data", 384, NULL, 1, NULL);
	xTaskCreate(smartmesh_parse_task, "Parse SMIP Data", 850, NULL, 1, NULL);
	vTaskStartScheduler(); // Start the scheduler
		
	for(;;); // SHOULD NOT REACH THIS POINT
}


extern "C"{
	// Hard fault interrupt handler function
	// Enters an infinite loop, effectively 
	// locking up the processor until it is reset.
	void HardFault_Handler(void){
        while(1){}
	}

	// Stack overflow hook function. Triggers an assertion failure
    void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
    {
		// 0 means the system is halted when a stack overflow is detected.
		configASSERT(0);
    }
};


/*
void __assert_func (const char * file, int, const char * line, const char * function)
{
	UART::getInst(SERCOM2)._printf("ASSERT at line %s, file %s, in function %s\r\n", file, line, function);
	configASSERT(0);
}
*/