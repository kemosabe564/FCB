//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMAND_H
#define QUADCOPTER_FCB_COMMAND_H

#include <stdint.h>

#define COMMAND_TYPE_MASK = 0b11111000

enum CommandType {
    SetControl = 0
};

struct CommandControlData {
    uint16_t yaw_rate;
    uint16_t roll_rate;
    uint16_t pitch_rate;
    uint16_t climb_rate;
};

struct Command {
    enum CommandType type;
    void *data;
};

struct Command *Command_create(uint8_t *data);
void Command_destroy(struct Command *self);

#endif //QUADCOPTER_FCB_COMMAND_H
