//
// Created by nathan on 28-04-21.
//

#include "CommandHandler.h"

#include "../hal/timers.h"
#include "Debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

struct CommandHandler *CommandHandler_create(uint8_t active_comms, CommandHandlerFunction handler)
{
    struct CommandHandler *result = (struct CommandHandler *)malloc(sizeof(struct CommandHandler));

    if (result)
    {
        timers_init();

        result->active_comm_id = active_comms;
        result->handler = handler;
        result->loop = LoopHandler_init_controlblock(CommandHandler_loop);
        result->heartbeat_seq = 0;
        result->heartbeat_ts = 0;
        result->on_heartbeat_lost = NULL;
        result->heartbeat_margin = 0;
        result->heartbeat_state = Dead;

        for (uint8_t i = 0; i < COMMANDHANDLER_MAX_COMMS; i += 1)
            result->comms[i] = NULL;
    }

    return result;
};

void CommandHandler_loop(void *context, uint32_t delta_us)
{
    struct CommandHandler *self = (struct CommandHandler *)context;
    struct Comms *comms = CommandHandler_get_active_comms(self);

    while (!CommandQueue_empty(&comms->receive_queue))
    {
        struct Command *command = CommandQueue_pop(&comms->receive_queue);

        // send back heartbeat acknowledge
        if (command->type == Heartbeat)
        {
            if (self->heartbeat_state == Dead)
            {
                self->heartbeat_state = Alive;
            }
            self->heartbeat_ts = get_time_us();
            uint8_t *seq = (uint8_t *)command->data;
            CommandHandler_send_command(self, Command_make_heartbeat(*seq));
        }

        self->handler(command);
        Command_destroy(command);
    }

    if (self->on_heartbeat_lost && (self->heartbeat_state == Alive) && ((get_time_us() - self->heartbeat_ts) >= self->heartbeat_margin))
    {
        self->heartbeat_state = Dead;
        self->on_heartbeat_lost();
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
            if (!CommandQueue_push(&comms->send_queue, command))
            {
                Command_destroy(command);
            }
        }
    }
}

void CommandHandler_set_on_heartbeat_lost(struct CommandHandler *self, uint32_t heartbeat_margin, CommandHandlerHeartbeatLostFunction handler)
{
    if (self)
    {
        self->heartbeat_margin = heartbeat_margin;
        self->on_heartbeat_lost = handler;
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