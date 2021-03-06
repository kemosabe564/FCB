//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMAND_H
#define QUADCOPTER_FCB_COMMAND_H

#include <stdint.h>
#include "LogData.h"
#define HEADER_TYPE_MASK    0b11110000
#define HEADER_SUBTYPE_MASK 0b00001111
#define HEADER_DATA_MASK    0b00001111

enum CommandType {
    Invalid = 0,
    SetOrQueryMode = 0b0001,
    CurrentMode = 0b0010,
    SetControl = 0b0011,
    AckControl = 0b0100,
    QueryTelemetry = 0b0101,
    CurrentTelemetry = 0b0110,
    DebugMessage = 0b0111,
    SetParam = 0b1000,
    AckParam = 0b1001,
    Heartbeat = 0b1010,
    SetComms = 0b1011,
    CurrentComms = 0b1100,
    Stop = 0b1101,
    LogInfo = 0b1111,
    LastCommand // Used for checking if command in valid range
};

struct CommandControlData {
    uint8_t yaw_rate;
    uint8_t pitch_rate;
    uint8_t roll_rate;
    uint8_t climb_rate;
};

struct CommandParamsData {
    uint8_t id;
    uint8_t value;
};

struct CommandDebugMessage {
    char *message;
    uint16_t size;
    uint8_t id;
};

struct CommandTelemetryData {
    int16_t roll_angle;
    int16_t pitch_angle;
    int16_t yaw_angle;
    uint16_t rpm[4];
};

struct Command {
    enum CommandType type;
    void *data;
};

struct EncodedCommand {
    uint8_t *data;
    uint16_t size;
};

struct Command *Command_decode(uint8_t *data);
struct EncodedCommand Command_encode(struct Command *command);
uint8_t Command_data_len(uint8_t header);

struct Command *Command_make_simple(enum CommandType type, uint8_t argument);
struct Command *Command_make_current_mode(uint8_t mode);
struct Command *Command_make_debug_format(uint8_t id, const char *format, ...);
struct Command *Command_make_debug_n(uint8_t id, const char *string, uint16_t n);
struct Command *Command_make_heartbeat(uint8_t sequence_number);
struct Command *Command_make_current_comms(uint8_t current_comms);
struct Command *Command_make_telemetry(int16_t roll_angle, int16_t pitch_angle, int16_t yaw_angle, int16_t rpm0, int16_t rpm1, int16_t rpm2, int16_t rpm3);

void Command_destroy(struct Command *self);

#endif //QUADCOPTER_FCB_COMMAND_H
