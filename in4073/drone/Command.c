//
// Created by nathan on 26-04-21.
//

#include "Command.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

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
            case SetOrQueryMode: {
                uint8_t *data = malloc(sizeof(uint8_t));
                *data = (header & HEADER_DATA_MASK);

                result->data = (void *)data;
            }
                break;
            case CurrentMode:
                break;
            case SetControl: {
                // Do something with the remaining data
                struct CommandControlData *data = (struct CommandControlData *)malloc(sizeof(struct CommandControlData));

                if (data)
                {
                    data->yaw_rate      = *((uint16_t *)&data[2]);
                    data->pitch_rate    = *((uint16_t *)&data[4]);
                    data->roll_rate     = *((uint16_t *)&data[6]);
                    data->climb_rate    = *((uint16_t *)&data[8]);
                }

                result->data = (void *)data;
            }
                break;
            case AckControl:
                break;
            case QueryForces:
                break;
            case CurrentForces:
                break;
            case DebugMsg:
                break;
            case SetParam:
                break;
            case AckParam:
                break;
            default:
                break;
        }
    }

    return result;
}

uint8_t *Command_encode(struct Command *command)
{
    uint8_t data_len = Command_data_len(command->type);
    uint8_t *encoded = NULL;

    switch (command->type)
    {
        case SetControl: {
            struct CommandControlData *d = (struct CommandControlData *)command->data;
            encoded = (uint8_t *)malloc((1 + data_len + 1) * sizeof(uint8_t));

            encoded[0] = (command->type << 4);
            encoded[1] = ((d->yaw_rate & 0xFF00) >> 8);
            encoded[2] = (d->yaw_rate & 0x00FF);
            encoded[3] = ((d->pitch_rate & 0xFF00) >> 8);
            encoded[4] = (d->pitch_rate & 0x00FF);
            encoded[5] = ((d->roll_rate & 0xFF00) >> 8);
            encoded[6] = (d->roll_rate & 0x00FF);
            encoded[7] = ((d->climb_rate & 0xFF00) >> 8);
            encoded[8] = (d->climb_rate & 0x00FF);

            encoded[9] = crc8_fast(encoded, 1 + data_len);
        }
            break;
        default:
            break;
    }

    return encoded;
}

uint8_t *Command_encode_set_mode(uint8_t mode)
{
    return (mode < 16) ? Command_encode_simple(SetOrQueryMode, mode) : NULL;
}

uint8_t *Command_encode_query_mode()
{
    return Command_encode_set_mode(0);
}
uint8_t *Command_encode_current_mode(uint8_t mode)
{
    return Command_encode_simple(CurrentMode, mode);
}

uint8_t *Command_encode_simple(enum CommandType type, uint8_t argument)
{
    uint8_t *command = (uint8_t *)malloc(2);

    if (command)
    {
        command[0] = (type << 4 | argument);
        command[1] = crc8_fast(command, 1);
    }

    return command;
}

//uint8_t *Command_encode_extended(enum CommandType type, uint8_t argument, uint8_t n);

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
            return 8;
        case AckControl:
            return 0;
        case QueryForces:
            return 0;
        case CurrentForces:
            return 6;
        case DebugMsg:
            return (header & HEADER_DATA_MASK);
        case SetParam:
            return 2;
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
            case SetControl: free((struct CommandControlData *) self->data); break;
            default:
                break;
        }

        free(self);
    }
}