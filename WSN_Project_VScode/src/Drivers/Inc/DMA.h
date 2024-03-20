#ifndef __DMA_H
#define __DMA_H

#include "saml21g18b.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define DESC_BASE_ADDR 0x30000000

typedef struct{
    uint16_t BTCTRL;
    uint16_t BTCNT;
    uint32_t SRCADDR;
    uint32_t DSTADDR;
    uint32_t DESCADDR;
}DMAC_descriptor_t;

class DMAC_Driver{
    public:
        static void DMAC_Driver_Init();
        static void DMAC_Send_Arr(uint8_t* data, uint16_t bytes, uint8_t channel);
        static void DMAC_Send_Arr(uint8_t* data, uint16_t bytes, uint8_t channel, uint32_t destAddr, bool fromISR = false);

    private:
        static volatile DMAC_descriptor_t *desc;
        static SemaphoreHandle_t DMAC_used;

        static BaseType_t giveSem(bool fromISR);
        static BaseType_t takeSem(bool fromISR);
};

#endif