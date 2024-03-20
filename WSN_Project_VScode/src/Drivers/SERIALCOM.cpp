#include "SERIALCOM.h"
#include "saml21g18b.h"
#include "DMA.h"

SemaphoreHandle_t UART::bluetoothIsUsed = (SemaphoreHandle_t)0;
UART_node_t UART::instances[SERCOM_NUM + 1];

UART::UART(Sercom *port, int baudrate, TickType_t semDelay){
	semDelayTicks = semDelay;
	UART_port = &port->USART;
	
	// Setup the proper SERCOM registers to enable UART
	// 16MHz clock used for all three SERCOMs
	if(port == SERCOM0){
		instances[0].SERCOM_inst = port;
		instances[0].class_inst = this;

		dma_channel_id = 1;
		PORT->Group[0].PINCFG[8].reg = 0x1;
		PORT->Group[0].PINCFG[10].reg = 0x1;
		PORT->Group[0].PMUX[4].reg = 0x2;
		PORT->Group[0].PMUX[5].reg = 0x2;
		NVIC->IP[2] |= 64;
		NVIC->ISER[0] |= (1 << 8);
		NVIC->ICPR[0] |= (1 << 8);
		GCLK->PCHCTRL[18].reg = 0x42;// Enable UART clock
	}else if(port == SERCOM1){
		instances[1].SERCOM_inst = port;
		instances[1].class_inst = this;

		dma_channel_id = 2;
		PORT->Group[0].PINCFG[16].reg = 0x1;
		PORT->Group[0].PINCFG[18].reg = 0x1;
		PORT->Group[0].PMUX[8].reg = 0x2;
		PORT->Group[0].PMUX[9].reg = 0x2;
		NVIC->IP[2] |= (64 << 8);
		NVIC->ISER[0] |= (1 << 9);
		NVIC->ICPR[0] |= (1 << 9);
		GCLK->PCHCTRL[19].reg = 0x42;// Enable UART clock
	}else if(port == SERCOM2){
		instances[2].SERCOM_inst = port;
		instances[2].class_inst = this;

		dma_channel_id = 3;
		PORT->Group[0].PINCFG[12].reg = 0x1;
		PORT->Group[0].PINCFG[14].reg = 0x1;
		PORT->Group[0].PMUX[6].reg = 0x2;
		PORT->Group[0].PMUX[7].reg = 0x2;
		NVIC->IP[2] |= (64 << 16);
		NVIC->ISER[0] |= (1 << 10);
		NVIC->ICPR[0] |= (1 << 10);
		GCLK->PCHCTRL[20].reg = 0x42;// Enable UART clock
	}

	// UART interface setup w/ proper baud rate
	UART_port->CTRLA.reg = 0x40010104;// Internal clock => SERCOM_PAD[2] TX, SERCOM_PAD[0] RX(SERCOM0 PA4 and PA6) //BUFFER OVERFLOW
	UART_port->CTRLB.reg |= 0x30000;// Enable tx and rx pins
	UART_port->BAUD.reg = 57999;// Always set to 115200 for now --> 57999
	//UART_port->DBGCTRL.bit.DBGSTOP = 1;
	UART_port->INTENSET.reg |= 0x4;// Enable interrupt rx complete interrupt
	UART_port->CTRLA.reg |= 0x2;// Enable the UART port

	if(bluetoothIsUsed == nullptr)
	{
		bluetoothIsUsed = xSemaphoreCreateBinary();
		xSemaphoreGive(bluetoothIsUsed);
	}
}

UART& UART::getInst(Sercom *sercom_used){
	for(int i = 0;i < SERCOM_NUM;i++)
	{
		if(instances[i].SERCOM_inst == sercom_used)
			return *instances[i].class_inst;
	}
	
	return *instances[6].class_inst;
}

int UART::_printf(const char *format, ...){
	configASSERT(UART::takeSem() == pdTRUE);
	va_list args;
	va_start(args, format);
	
	char buffer[UART_BUFFER_SIZE];
	int done = vsprintf(buffer, format, args);
	
	if(done > UART_BUFFER_SIZE)
		return 0;
	
	va_end(args);

	DMAC_Driver::DMAC_Send_Arr((uint8_t*)buffer, done, dma_channel_id);

	configASSERT(UART::giveSem() == pdTRUE);
	
	return 0;
}

void UART::send_array(uint8_t *data, uint8_t length){
	configASSERT(UART::takeSem() == pdTRUE);

	DMAC_Driver::DMAC_Send_Arr(data, length, dma_channel_id);

	configASSERT(UART::giveSem() == pdTRUE);
}

inline bool UART::isInstance(UART *inst)
{
	return (inst == this);
}

BaseType_t UART::giveSem()
{
	return xSemaphoreGive(bluetoothIsUsed);
}

BaseType_t UART::takeSem()
{
	return xSemaphoreTake(bluetoothIsUsed, 1000);
}
