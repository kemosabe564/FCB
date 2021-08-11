#ifndef QUADCOPTER_FCB_LOGHANDLER_H
#define QUADCOPTER_FCB_LOGHANDLER_H
#include "LogData.h"
#include "../hal/spi_flash.h"
#include "CommandHandler.h"
#include "FlightController.h"
#include <stdint.h>
#include <stdlib.h>
//Authored by Yuxiang
struct LogHandler{
    struct CommandHandler *ch;
    uint32_t start_add;
    uint32_t curr_add;
    struct FlightController * flc;
    struct LoopHandlerControlBlock loop;
};

struct LogHandler * LogHandler_create(struct CommandHandler *ch, uint32_t init_add,struct FlightController * fc);
void LogHandler_append(struct LogHandler * self, struct LogData * new_logData);
struct LogData * LogHandler_dequeue(struct LogHandler * self);
struct LogData * LogHandler_pop(struct LogHandler * self);
void LogHandler_loop(void * context, uint32_t delta_us);
void LogHandler_destroy(struct LogHandler * self);
#endif
