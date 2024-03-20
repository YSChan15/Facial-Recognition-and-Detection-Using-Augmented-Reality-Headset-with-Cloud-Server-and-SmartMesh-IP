#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include <stdint.h>
#include "Freertos.h"
#include "queue.h"
#include "task.h"
#include "SERCOM.h"
#include "API.h"

extern Smartmesh_API api;
extern uint8_t smartmeshData[130];

void bluetoothParse(void* unused);

#endif
