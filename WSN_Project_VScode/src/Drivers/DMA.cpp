#include "DMA.h"

volatile DMAC_descriptor_t *DMAC_Driver::desc = (volatile DMAC_descriptor_t*)DESC_BASE_ADDR;
SemaphoreHandle_t DMAC_Driver::DMAC_used = xSemaphoreCreateBinary();

void DMAC_Driver::DMAC_Driver_Init()
{
    desc[0].BTCTRL     = 0x0C01;// Channel 0 descriptor
    desc[0].BTCNT      = 0x0100;
    desc[0].SRCADDR    = (uint32_t)&SERCOM0->USART.BAUD.reg;
	desc[0].DSTADDR    = 0x30000100;
	desc[0].DESCADDR   = 0x00000000;

	desc[1].BTCTRL     = 0x0401;// Channel 1 descriptor
    desc[1].BTCNT      = 0x0100;
	desc[1].SRCADDR    = 0x30000000;
	desc[1].DSTADDR    = (uint32_t)&SERCOM0->USART.DATA.reg;
	desc[1].DESCADDR   = 0x00000000;

	desc[2].BTCTRL     = 0x0401;// Channel 2 descriptor
    desc[2].BTCNT      = 0x0100;
	desc[2].SRCADDR    = 0x30000000;
	desc[2].DSTADDR    = (uint32_t)&SERCOM1->USART.DATA.reg;
	desc[2].DESCADDR   = 0x00000000;

	desc[3].BTCTRL     = 0x0401;// Channel 3 descriptor
    desc[3].BTCNT      = 0x0100;
	desc[3].SRCADDR    = 0x30000000;
	desc[3].DSTADDR    = (uint32_t)&SERCOM2->USART.DATA.reg;
	desc[3].DESCADDR   = 0x00000000;

	desc[4].BTCTRL     = 0x0C01;// Channel 4 descriptor
    desc[4].BTCNT      = 0x0100;
	desc[4].SRCADDR    = (uint32_t)&SERCOM0->USART.BAUD.reg;
	desc[4].DSTADDR    = 0x30000100;
	desc[4].DESCADDR   = 0x00000000;

    DMAC->BASEADDR.reg = DESC_BASE_ADDR;
	DMAC->WRBADDR.reg = DESC_BASE_ADDR + 0x100;
	DMAC->CTRL.reg = 0xF02;
	DMAC->CHCTRLB.reg = 0x60;// Channel 0 config
	DMAC->CHID.reg = 0x1;// Set to channel 1
	DMAC->CHCTRLB.reg = 0x800260;
	DMAC->CHID.reg = 0x2;// Set to channel 2
	DMAC->CHCTRLB.reg = 0x800460;
	DMAC->CHID.reg = 0x3;// Set to channel 3
	DMAC->CHCTRLB.reg = 0x800660;
	DMAC->CHID.reg = 0x4;// Set to channel 4
	DMAC->CHCTRLB.reg = 0x60;

    giveSem(false);
}

void DMAC_Driver::DMAC_Send_Arr(uint8_t* data, uint16_t bytes, uint8_t channel)
{
    configASSERT(takeSem(false) == pdTRUE); // Wait for dma regs to become available
	DMAC->CHID.reg = channel; // Setup the dma transfer
	while(DMAC->CHCTRLA.reg != 0); // Check to see if DMA is still running
    desc[channel].BTCNT = bytes;
    desc[channel].SRCADDR = (uint32_t)data + (uint32_t)bytes; // Source address end value
	DMAC->CHCTRLA.reg = 0x2; // Enable the channel
	configASSERT(giveSem(false) == pdTRUE); // Give back the semphore as done accessing the data
}

void DMAC_Driver::DMAC_Send_Arr(uint8_t* data, uint16_t bytes, uint8_t channel, uint32_t destAddr, bool fromISR)
{
	configASSERT(takeSem(fromISR) == pdTRUE); // Wait for dma regs to become available
	DMAC->CHID.reg = channel; // Setup the dma transfer
	while(DMAC->CHCTRLA.reg != 0); // Check to see if DMA is still running
    desc[channel].BTCNT = bytes;
    desc[channel].SRCADDR = (uint32_t)data + (uint32_t)bytes; // Source address end value
	desc[channel].DSTADDR = (uint32_t)destAddr + (uint32_t)bytes;
	DMAC->CHCTRLA.reg = 0x2; // Enable the channel
	DMAC->SWTRIGCTRL.reg |= 0x1;
	configASSERT(giveSem(fromISR) == pdTRUE); // Give back the semphore as done accessing the data
}

BaseType_t DMAC_Driver::giveSem(bool fromISR)
{
	return !fromISR ? xSemaphoreGive(DMAC_used) : xSemaphoreGiveFromISR(DMAC_used, nullptr);
}

BaseType_t DMAC_Driver::takeSem(bool fromISR)
{
	return !fromISR ? xSemaphoreTake(DMAC_used, portMAX_DELAY) : xSemaphoreTakeFromISR(DMAC_used, nullptr);
}
