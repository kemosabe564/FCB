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
            case CurrentMode: // Doesn't need to be decoded on drone side
                break;
            case SetControl: {
                // Do something with the remaining data
                struct CommandControlData *control_data = (struct CommandControlData *)malloc(sizeof(struct CommandControlData));

                if (data)
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
    uint8_t *encoded = NULL;

    switch (command->type)
    {
        case CurrentMode: {
            encoded = (uint8_t *)malloc((1 + 0 + 1) * sizeof(uint8_t));

            uint8_t *mode = (uint8_t *)command->data;
            encoded[0] = ((command->type << 4) | *mode);

            encoded[1] = crc8_fast(encoded, 1);
        }
            break;
        default:
            break;
    }

    return encoded;
}

uint8_t *Command_encode_current_mode(uint8_t mode)
{
    struct Command cmd = {
        .type = CurrentMode,
        .data = (void *) &mode
    };

    return Command_encode(&cmd);
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
            case SetOrQueryMode: free((uint8_t *) self->data); break;
            case SetControl: free((struct CommandControlData *) self->data); break;
            default:
                break;
        }

        free(self);
    }
}