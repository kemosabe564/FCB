//
// Created by nathan on 28-04-21.
//

#include "CommandHandler.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

struct CommandHandler *CommandHandler_create(uint8_t active_comms, CommandHandlerFunction handler)
{
    struct CommandHandler *result = (struct CommandHandler *)malloc(sizeof(struct CommandHandler));

    if (result)
    {
        result->active_comm_id = active_comms;
        result->handler = handler;
        result->loop = LoopHandler_init_controlblock(CommandHandler_loop);

        for (uint8_t i = 0; i < COMMANDHANDLER_MAX_COMMS; i += 1)
            result->comms[i] = NULL;
    }

    return result;
};

void CommandHandler_loop(void *context, uint32_t delta_us)
{
    struct CommandHandler *self = (struct CommandHandler *)context;
    struct Comms *comms = CommandHandler_get_active_comms(self);

    if(!CommandQueue_empty(&comms->receive_queue))
    {
        struct Command *command = CommandQueue_pop(&comms->receive_queue);
        self->handler(command);
        Command_destroy(command);
    }
};

void CommandHandler_add_comms(struct CommandHandler *self, uint8_t id, struct Comms *comms)
{
    if (self && id < COMMANDHANDLER_MAX_COMMS)
    {
        self->comms[id] = comms;
    }
};

struct Comms *CommandHandler_get_active_comms(struct CommandHandler *self)
{
    if (self)
    {
        return self->comms[self->active_comm_id];
    }

    return NULL;
}

void CommandHandler_send_command(struct CommandHandler *self, struct Command *command)
{
    if (self && command)
    {
        struct Comms *comms = CommandHandler_get_active_comms(self);

        if (comms)
        {
            CommandQueue_push(&comms->send_queue, command);
        }
    }
}

void CommandHandler_destroy(struct CommandHandler *self)
{
    if (self)
    {
        // Also empty queues...
        free(self);
    }
}