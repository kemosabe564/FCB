//
// Created by nathan on 26-04-21.
//

#include "Command.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "utils/crc8.h"

struct Command *Command_decode(uint8_t *data)
{
    const uint8_t header = data[0];
    uint8_t type = ((header & HEADER_TYPE_MASK) >> 4);

    if (type >= LastCommand)
        type = Invalid;

    uint8_t data_len = Command_data_len(header);
    uint8_t crc = data[1 + data_len]; // header size + data length

    if (!crc8_fast_compare(crc, data, 1 + data_len))
        type = Invalid;

    struct Command *result = (struct Command *)malloc(sizeof(struct Command));

    if (result)
    {
        result->type = type;
        switch (type)
        {
            case Invalid:
                result->data = NULL;
                break;
            case SetOrQueryMode:
            case CurrentMode: {
                uint8_t *argument = malloc(sizeof(uint8_t));
                *argument = (header & HEADER_DATA_MASK);

                result->data = (void *)argument;
            }
                break;
            case SetControl: {
                // Do something with the remaining data
                struct CommandControlData *control_data = (struct CommandControlData *)malloc(sizeof(struct CommandControlData));

                if (control_data)
                {
                    control_data->yaw_rate      = data[1];
                    control_data->pitch_rate    = data[2];
                    control_data->roll_rate     = data[3];
                    control_data->climb_rate    = data[4];
                }

                result->data = (void *)control_data;
            }
                break;
            case AckControl:
                break;
            case QueryForces:
                break;
            case CurrentForces:
                break;
            case DebugMessage:
                break;
            case SetParam:{
                struct CommandParamsData *paramsData = (struct CommandParamsData *)malloc(sizeof(struct CommandParamsData));

                if (paramsData)
                {
                    paramsData->id = (header & HEADER_DATA_MASK);
                    paramsData->value = data[1];
                }
                result->data = (void *) paramsData;
            }
                break;
            case AckParam:
                break;
            default:
                break;
        }
    }

    return result;
}

struct EncodedCommand Command_encode(struct Command *command)
{
    uint8_t *encoded = NULL;
    uint16_t size = 0;

    switch (command->type)
    {
        case CurrentMode:
        case Heartbeat: { // encoding logic of the simple argumented command type
            size = (1 + 0 + 1);
            encoded = (uint8_t *)malloc(size * sizeof(uint8_t));

            uint8_t *argument = (uint8_t *)command->data;
            encoded[0] = ((command->type << 4) | *argument);

            encoded[1] = crc8_fast(encoded, 1);
        }
            break;
        case DebugMessage: {
            struct CommandDebugMessage *data = (struct CommandDebugMessage *)command->data;
            uint16_t message_size = data->size;

            // protocol only allows for 256 characters per debug command
            if (message_size > 17)
                message_size = 17;

            // allocate space for encoded command string
            size = (1 + message_size + 1); // header + size of message + [message] + crc
            encoded = (uint8_t *)malloc(size * sizeof(uint8_t));

            // set header
            encoded[0] = ((command->type << 4) | ((message_size - 1) & 0b1111));
            // copy debug message to the remaining buffer
            memcpy(&encoded[1], data->message, message_size);

            // setting last buffer element to CRC
            encoded[size - 1] = crc8_fast(encoded, size - 1);
        }
            break;
        default:
            break;
    }

    struct EncodedCommand handle = {
        .data = encoded,
        .size = size
    };

    return handle;
}

// allocate and initialize a simple argumented command on the heap
struct Command *Command_make_simple(enum CommandType type, uint8_t argument)
{
    struct Command *cmd = (struct Command *)malloc(sizeof(struct Command));

    // Only continue if cmd != NULL
    // which means malloc was successful
    if (cmd)
    {
        cmd->type = type;

        uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t));

        // same as with previous malloc
        if (data)
        {
            // allocation successful, so set data!
            *data = argument;
            cmd->data = (void *)data;

            return cmd;
        }

        // since second malloc failed, free previously allocated memory
        free(cmd);
    }

    return NULL;
}

// alias for Command_make_simple
struct Command *Command_make_heartbeat(uint8_t sequence_number)
{
    return Command_make_simple(Heartbeat, sequence_number);
}

// alias for Command_make_simple
struct Command *Command_make_current_mode(uint8_t mode)
{
    return Command_make_simple(CurrentMode, mode);
}

// allocate and initialize a formatted string command on the heap
struct Command *Command_make_debug_format(const char *format, ...)
{
    struct Command *cmd = (struct Command *)malloc(sizeof(struct Command));

    if (cmd)
    {
        cmd->type = DebugMessage;
        struct CommandDebugMessage *data = (struct CommandDebugMessage *)malloc(sizeof(struct CommandDebugMessage));

        if (data)
        {
            va_list args;
            va_start(args, format);
            uint16_t size = vsprintf(NULL, format, args);

            if (size > 0)
            {
                char *message = (char *)malloc(size * sizeof(char));
                vsprintf(message, format, args);

                data->size = size;
                data->message = message;

                cmd->data = (void *)data;

                return cmd;
            }

            // Allocation of message failed
            free(data);
        }

        // Allocation of data struct failed
        free(cmd);
    }

    return NULL;
}

// allocate and initialize a formatted string command on the heap
struct Command *Command_make_debug_n(const char *string, uint16_t n)
{
    struct Command *cmd = (struct Command *)malloc(sizeof(struct Command));

    if (cmd)
    {
        cmd->type = DebugMessage;
        struct CommandDebugMessage *data = (struct CommandDebugMessage *)malloc(sizeof(struct CommandDebugMessage));

        if (data)
        {
            char *message = (char *)malloc(n * sizeof(char));

            if (message)
            {
                memcpy(message, string, n);

                data->size = n;
                data->message = message;

                cmd->data = (void *)data;

                return cmd;
            }

            // Allocation of message failed
            free(data);
        }

        // Allocation of data struct failed
        free(cmd);
    }

    return NULL;
}

uint8_t Command_data_len(uint8_t header)
{
    switch ((header & HEADER_TYPE_MASK) >> 4)
    {
        case Invalid:
            return 0;
        case SetOrQueryMode:
            return 0;
        case CurrentMode:
            return 0;
        case SetControl:
            return 4;
        case AckControl:
            return 0;
        case QueryForces:
            return 0;
        case CurrentForces:
            return 6;
        case DebugMessage:
            return (header & HEADER_DATA_MASK);
        case SetParam:
            return 1;
        case AckParam:
            return 0;
        case LastCommand:
            return 0;
        default:
            return 0;
    }
}

void Command_destroy(struct Command *self)
{
    if (self)
    {
        switch (self->type)
        {
            case SetOrQueryMode:
            case CurrentMode:
                free((uint8_t *) self->data);
                break;
            case SetControl:
                free((struct CommandControlData *) self->data);
                break;
            case DebugMessage: {
                struct CommandDebugMessage *data = (struct CommandDebugMessage *)self->data;
                free(data->message);
                free(data);
            }
                break;
            default:
                break;
        }

        free(self);
    }
}