#ifndef QUADCOPTER_FCB_LOGHANDLERFLUSH_H
#define QUADCOPTER_FCB_LOGHANDLERFLUSH_H
#include "LogData.h"
#include "LogHandler.h"
#include "../hal/spi_flash.h"
#include "CommandHandler.h"
#include "FlightController.h"
#include <stdint.h>
#include <stdlib.h>
//Authored by Yuxiang
struct LogHandler_flush{
    struct LogHandler *lgh;
    struct LoopHandlerControlBlock loop;
};

struct LogHandler_flush * LogHandler_flush_create(struct LogHandler *lgh);
void LogHandler_flush_loop(void * context, uint32_t delta_us);
void LogHandler_flush_destroy(struct LogHandler_flush * self);
#endif