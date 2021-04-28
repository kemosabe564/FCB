//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMAND_H
#define QUADCOPTER_FCB_COMMAND_H

#include <stdint.h>

#define S_CONTROLS 1

#define COMMAND_TYPE_MASK = 0b11111000

struct Command_Type_Control {
    uint16_t yaw_rate;
    uint16_t roll_rate;
    ....
};

struct Command {
    uint8_t type;
    void *data;
};

Control = yaw, pitch, roll, thrust etc
Change communication = serial, wireless
Change mode = yaw, height, manual
Logging level = debug, normal
Enable logging = start, stop

struct command Command_make(uint8_t *data);

#endif //QUADCOPTER_FCB_COMMAND_H
