#include "LogHandler.h"
#include "../hal/timers.h"
#include "../../components/drivers_nrf/hal/nrf_gpio.h"
#include "../hal/gpio.h"
#include "../hal/timers.h"
#include "../mpu6050/mpu6050.h"
#include "../hal/uart.h"
#include "utils/crc8.h"
#include "Command.h"
#include "CommandHandler.h"
#include <stdlib.h>
#include <stdint.h>
#define MEM_UPPERLIMIT 0x01FFFF

//Authored by Yuxiang
struct LogHandler * LogHandler_create(struct CommandHandler *ch,uint32_t init_add, struct FlightController * fc){
    struct LogHandler * result = (struct LogHandler *) malloc (sizeof(struct LogHandler));
    if(result){
        result->ch = ch;
        result->start_add = init_add;
        result->curr_add = result->start_add;  
        result->flc = fc;
        result->loop = LoopHandler_init_controlblock(LogHandler_loop);
    }
    return result;
}

void LogHandler_append(struct LogHandler * self, struct LogData * new_logData){
    if(self->curr_add + sizeof(struct LogData) < MEM_UPPERLIMIT){
        flash_write_bytes(self->curr_add, (uint8_t *)new_logData, sizeof(struct LogData));
        self->curr_add += sizeof(struct LogData);
    }
}

struct LogData * LogHandler_dequeue(struct LogHandler * self){
    struct LogData * result = LogData_create();
      if(self->start_add + sizeof(struct LogData) <= self->curr_add){
        flash_read_bytes(self->start_add, (uint8_t *) result, sizeof(struct LogData));
        self->start_add += sizeof(struct LogData);
    }
    return result;
}

struct LogData * LogHandler_pop(struct LogHandler * self){
    struct LogData * result = LogData_create();
      if(self->curr_add - sizeof(struct LogData) >= self->start_add){
        self->curr_add -= sizeof(struct LogData);
        flash_read_bytes(self->curr_add, (uint8_t *) result, sizeof(struct LogData));
    }
    return result;
}

void LogHandler_loop(void * context, uint32_t delta_us){
    struct LogHandler * self  = (struct LogHandler *) context;
    struct LogData * data =  LogData_create();
    
    data->psi = 11;
    data->phi = 22;
    data->theta = 33;

    /*
    data->motor0_rpm = self->flc->rotors[0]->actual_rpm;
    data->motor1_rpm = self->flc->rotors[1]->actual_rpm;
    data->motor2_rpm = self->flc->rotors[2]->actual_rpm;
    data->motor3_rpm = self->flc->rotors[3]->actual_rpm;
    */

    data->motor0_rpm = 44;
    data->motor1_rpm = 55;
    data->motor2_rpm = 66;
    data->motor3_rpm = 77;

    nrf_gpio_pin_toggle(RED);
    LogHandler_append(self, data);
    LogData_destroy(data);
}

void LogHandler_destroy(struct LogHandler * self){
    if(self){
        free(self);
    }
}