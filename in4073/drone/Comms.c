//
// Created by nathan on 26-04-21.
//

#include "Comms.h"

#include <stddef.h>

#include <stdio.h>

void Comms_enqueue_command(struct Comms* self, struct Command *command)
{
    if (self && command)
    {
        self->last = (self->last + 1) % COMMS_QUEUE_LEN;
        self->commands[self->last] = command;
        self->count += 1;
    }
}

struct Command *Comms_dequeue_command(struct Comms* self)
{
    if (self)
    {
        struct Command *first = self->commands[self->first];
        self->first = (self->first + 1) % COMMS_QUEUE_LEN;
        self->count -= 1;

        return first;
    }

    return NULL;
}

bool Comms_available(struct Comms* self)
{
    if (self)
    {
        return self->count > 0;
    }

    return false;
}
