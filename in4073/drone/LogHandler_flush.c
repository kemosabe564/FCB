#include "LogHandler.h"
#include "LogHandler_flush.h"
#include "../hal/gpio.h"
#include "../hal/timers.h"
#include "../mpu6050/mpu6050.h"
#include "../hal/uart.h"
#include "utils/crc8.h"
#include "Command.h"
#include "CommandHandler.h"
#include <stdlib.h>
#include <stdint.h>

struct LogHandler_flush * LogHandler_flush_create(struct LogHandler *lgh){
    struct LogHandler_flush * result = (struct LogHandler_flush *) malloc (sizeof(struct LogHandler_flush));
    if(result){
        result->lgh = lgh;
        result->loop = LoopHandler_init_controlblock(LogHandler_flush_loop);
    }
    return result;
}
void LogHandler_flush_loop(void * context, uint32_t delta_us){
    struct LogHandler_flush * self  = (struct LogHandler_flush *) context;

    struct LogData * data_entry = LogData_create();
    
    if(data_entry){
        data_entry = LogHandler_dequeue(self->lgh);
        struct Command *cmd = Command_make_LogData(data_entry);
        CommandHandler_send_command(self->lgh->ch, cmd);
        free(data_entry);
    }
}
void LogHandler_flush_destroy(struct LogHandler_flush * self){
    if(self){
        free(self);
    }
}