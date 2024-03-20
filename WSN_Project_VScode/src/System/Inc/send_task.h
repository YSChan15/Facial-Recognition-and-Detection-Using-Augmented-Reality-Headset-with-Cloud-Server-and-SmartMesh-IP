#ifndef __SEND_TASK_H
#define __SEND_TASk_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

// LTE packet struct
typedef struct{
    uint8_t *data;
    uint64_t mac_address;
}packet_t;

// LTE packet queue containing packets to send. Can contain upto 10 packets
static QueueHandle_t lte_packet_queue = xQueueCreate(10, sizeof(packet_t));

// Task to send packets to the LTE module
void send_task(void *unused);

// Function to add packets to lte_packet_queue
void enqueue_packet(packet_t lte_packet);

#endif
