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
    enum CommandType type = (enum CommandType)data[0];
    uint8_t crc = data[1];

    if (type >= LastCommand)
        type = Invalid;

    if (!crc8_fast_check(crc, &data[2], Command_data_len(type)))
        type = Corrupted;

    struct Command *result = (struct Command *)malloc(sizeof(struct Command));

    if (result)
    {
        result->type = type;
        switch (type)
        {
            case SetControl: {
                // Do something with the remaining data
                struct CommandControlData *data = (struct CommandControlData *)malloc(sizeof(struct CommandControlData));

                if (data)
                {
//                    data->yaw_rate      = ((data[2] << 8) & data[3]);
//                    data->pitch_rate    = ((data[4] << 8) & data[5]);
//                    data->roll_rate     = ((data[6] << 8) & data[7]);
//                    data->climb_rate    = ((data[8] << 8) & data[9]);

                    data->yaw_rate      = *((uint16_t *)&data[2]);
                    data->pitch_rate    = *((uint16_t *)&data[4]);
                    data->roll_rate     = *((uint16_t *)&data[6]);
                    data->climb_rate    = *((uint16_t *)&data[8]);
                }

                result->data = (void *)data;
            }
                break;
            default:
                break;
        }
    }

    return result;
}

uint8_t *Command_encode(enum CommandType type, void *data)
{
    uint8_t data_len = Command_data_len(type);
    uint8_t *encoded = NULL;

    switch (type)
    {
        case SetControl: {
            struct CommandControlData *d = (struct CommandControlData *)data;
            encoded = (uint8_t *)malloc((2 + data_len + 1) * sizeof(uint8_t));

            encoded[0] = type;
            encoded[2] = (d->yaw_rate & 0xFF00 >> 8);
            encoded[3] = (d->yaw_rate & 0x00FF);
            encoded[4] = (d->pitch_rate & 0xFF00 >> 8);
            encoded[5] = (d->pitch_rate & 0x00FF);
            encoded[6] = (d->roll_rate & 0xFF00 >> 8);
            encoded[7] = (d->roll_rate & 0x00FF);
            encoded[8] = (d->climb_rate & 0xFF00 >> 8);
            encoded[9] = (d->climb_rate & 0x00FF);
            encoded[10] = '\n';

            encoded[1] = crc8_fast(&encoded[2], data_len);
        }
            break;
        default:
            break;
    }

    return encoded;
}

uint8_t Command_data_len(enum CommandType type)
{
    switch (type) {
        case SetModeSafe:
        case AckModeSafe:
        case SetModePanic:
        case AckModePanic:
        case SetModeManual:
        case AcKModeManual:
            return 0;
        case SetControl:
            return 8;
        case LastCommand:
        case Invalid:
        case Corrupted:
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