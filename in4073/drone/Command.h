//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMAND_H
#define QUADCOPTER_FCB_COMMAND_H

#include <stdint.h>

#define COMMAND_TYPE_MASK 0b11111111

enum CommandType {
    SetModeSafe     = 0,
    AckModeSafe,
    SetModePanic,
    AckModePanic,
    SetModeManual,
    AcKModeManual,
    SetControl,
    LastCommand,
    Invalid         = 254,
    Corrupted       = 255,
};

struct CommandControlData {
    uint16_t yaw_rate;
    uint16_t pitch_rate;
    uint16_t roll_rate;
    uint16_t climb_rate;
};

struct Command {
    enum CommandType type;
    void *data;
};

struct Command *Command_decode(uint8_t *data);
uint8_t Command_data_len(enum CommandType type);
uint8_t *Command_encode(enum CommandType type, void *data);
void Command_destroy(struct Command *self);

#endif //QUADCOPTER_FCB_COMMAND_H
