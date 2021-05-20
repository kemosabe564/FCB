//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMAND_H
#define QUADCOPTER_FCB_COMMAND_H

#include <stdint.h>

#define HEADER_TYPE_MASK    0b11110000
#define HEADER_SUBTYPE_MASK 0b00001111
#define HEADER_DATA_MASK    0b00001111

enum CommandType {
    Invalid = 0,
    SetOrQueryMode,
    CurrentMode,
    SetControl,
    AckControl,
    QueryForces,
    CurrentForces,
    DebugMsg,
    SetParam,
    AckParam,
    LastCommand // Used for checking if command in valid range
};

struct CommandControlData {
    uint8_t yaw_rate;
    uint8_t pitch_rate;
    uint8_t roll_rate;
    uint8_t climb_rate;
};

struct Command {
    enum CommandType type;
    void *data;
};

struct Command *Command_decode(uint8_t *data);
uint8_t Command_data_len(uint8_t header);

uint8_t *Command_encode(struct Command *command);
uint8_t *Command_encode_current_mode(uint8_t mode);

void Command_destroy(struct Command *self);

#endif //QUADCOPTER_FCB_COMMAND_H
