#ifndef __SM_PARSE_H
#define __SM_PARSE_H

#define API_FLAG    0x7E

typedef enum{
    STATE_INIT,
    STATE_IDLE,
    STATE_START_FOUND,
    STATE_END_FOUND,
    STATE_ERROR,

    STATE_AMT
}states;

typedef uint8_t packet_type_t;

void smartmesh_parse_task(void *unused);
bool mgr_connection_status(void);
BaseType_t get_packet_status(packet_type_t packet);

#endif
