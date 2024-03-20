#include "send_task.h"
#include "lte_parse.h"
#include "common.h"

// This task sends one packet to the LTE module
void send_task(void *unused)
{
    // The packet to send
    packet_t curr_packet;
    while (1)
    {
        // Get one packet from the queue
        xQueueReceive(lte_packet_queue, &curr_packet, portMAX_DELAY);

        //debug AT commands
        //lte.checkDebugInfo();

        // Convert mac address to appropriate format and send the packet
        //lte.send_to_firebase(curr_packet.data, (uint8_t*)convertMacAddress((const char*)&curr_packet.mac_address));
    }
    vTaskDelete(nullptr); // Should not reach this point
}

// Adds a data packet to the queue.
void enqueue_packet(packet_t lte_packet)
{
    // Attempt to add to queue for 1000ms and hardfault if not possible
    configASSERT(xQueueSend(lte_packet_queue, &lte_packet, 1000) == pdTRUE);
}