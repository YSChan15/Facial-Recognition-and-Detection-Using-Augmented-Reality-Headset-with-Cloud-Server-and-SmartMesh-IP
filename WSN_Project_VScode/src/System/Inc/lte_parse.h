// #ifndef __LTE_PARSE_H
// #define __LTE_PARSE_H

#pragma once

#include <string>
#include <stdint.h>
#include <cassert>
#include "Freertos.h"
#include "queue.h"
#include "task.h"
#include "SERIALCOM.h"
#include "LTE.h"

#define LTE_BUFFER_SIZE 300
#define AT_HEAD "AT"
#define AT_SUCCESS "OK"
#define AT_ERROR "ERROR"
#define AT_CLOSED "CLOSED"
#define AT_CME_ERROR "+CME ERROR:"

typedef enum
{
    AT_COMMAND_READ,
    AT_COMMAND_WRITE,
    AT_COMMAND_TEST,
    AT_COMMAND_RUN,
		AT_COMMAND_UNKNOWN
} cmd_type_t;

// Declare LTE class instance as global
extern LTE lte;

// LTE module tasks
void beginLTE(void* unused);
void lte_parse_task(void* unused);



// #endif

/*
#pragma once

class LTE_Parse {
  private:
      static LTE_Parse * s_instance;

  public:
      static LTE_Parse* GetInstance() { static LTE_Parse parse; return &parse; };
      LTE_Parse();

      void Task(); // task code

    
};

#ifdef __cplusplus
extern "C"{
#endif

void LTE_Parse_Task(void *unsued);

#ifdef __cplusplus
};
#endif
*/