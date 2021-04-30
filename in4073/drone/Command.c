//
// Created by nathan on 26-04-21.
//

#include "Command.h"
#include <stdlib.h>

#include <stdio.h>

struct Command *Command_create(uint8_t *data)
{
//    uint8_t type = (data[0] && COMMAND_TYPE_MASK) >> 3;
    enum CommandType type = (enum CommandType) data[0];

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
                    data->yaw_rate = 101;
                    data->roll_rate = 102;
                    data->pitch_rate = 103;
                    data->climb_rate = 104;
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