#ifndef QUADCOPTER_FCB_LOG_INFO_H
#define QUADCOPTER_FCB_LOG_INFO_H

#include <stdint.h>
#include <stdbool.h>
//Authored by Yuxiang
struct LogData{   
    int16_t psi;
    int16_t phi;
    int16_t theta;
    
    int16_t motor0_rpm;
    int16_t motor1_rpm;
    int16_t motor2_rpm;
    int16_t motor3_rpm;
};

struct LogData * LogData_create();
struct Command *Command_make_LogData(struct LogData * data_entry);
void LogData_destroy(struct LogData *);
#endif
