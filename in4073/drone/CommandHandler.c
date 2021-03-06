//
// Created by nathan on 28-04-21.
//

#include "CommandHandler.h"

#include "../hal/timers.h"
#include "Debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
//authored by Nathan
struct CommandHandler *CommandHandler_create(uint8_t active_comms, CommandHandlerFunction handler)
{
    struct CommandHandler *result = (struct CommandHandler *)malloc(sizeof(struct CommandHandler));

    if (result)
    {
        timers_init();

        result->active_comm_id = active_comms;
        result->handler = handler;
        result->loop = LoopHandler_init_controlblock(CommandHandler_loop);
        result->state = CH_Init;
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
//authored by Nathan
void CommandHandler_loop(void *context, uint32_t delta_us)
{
    struct CommandHandler *self = (struct CommandHandler *)context;

    switch (self->state)
    {
    case CH_Init: {
        for (uint8_t i = 0; i < COMMANDHANDLER_MAX_COMMS; i += 1)
        {
            if (self->comms[i] != NULL)
            {
                CommandQueue_push(&self->comms[i]->send_queue, Command_make_current_comms(self->active_comm_id));
            }
        }

        self->state = CH_Running;
    }
        break;
    case CH_Running: {
        struct Comms *comms = CommandHandler_get_active_comms(self);

        while (!CommandQueue_empty(&comms->receive_queue))
        {
            struct Command *command = CommandQueue_pop(&comms->receive_queue);

            __CommandHandler_command_handler_internal(self, command);

            self->handler(command);
            Command_destroy(command);
        }

        int32_t heartbeat_diff = (get_time_us() - self->heartbeat_ts);
        if (self->on_heartbeat_lost && (self->heartbeat_state == Alive) && (heartbeat_diff > 0) && (heartbeat_diff >= self->heartbeat_margin))
        {
            self->heartbeat_state = Dead;

            DEBUG(0, "HB: %d, %d, diff: %d", self->heartbeat_ts, get_time_us(), (get_time_us() - self->heartbeat_ts));
            self->on_heartbeat_lost();
        }
    }
        break;
    default:
        break;
    }
};
//authored by Nathan
void __CommandHandler_command_handler_internal(struct CommandHandler *self, struct Command *command)
{
    switch (command->type)
    {
    case Heartbeat: {
        if (self->heartbeat_state == Dead)
        {
            self->heartbeat_state = Alive;
        }
        self->heartbeat_ts = get_time_us();
        uint8_t *seq = (uint8_t *)command->data;
        CommandHandler_send_command(self, Command_make_heartbeat(*seq));
    }
        break;
    case SetComms: {
        uint8_t *idx = (uint8_t *)command->data;

        if (*idx < COMMANDHANDLER_MAX_COMMS && *idx != self->active_comm_id)
        {
            CommandHandler_send_command(self, Command_make_current_comms(*idx)); // send new comm to current comm
            self->active_comm_id = *idx;
        }
    }
        break;
    default:
        break;
    }
}
//authored by Nathan
void CommandHandler_add_comms(struct CommandHandler *self, uint8_t id, struct Comms *comms)
{
    if (self && id < COMMANDHANDLER_MAX_COMMS)
    {
        self->comms[id] = comms;
    }
};
//authored by Nathan
struct Comms *CommandHandler_get_active_comms(struct CommandHandler *self)
{
    if (self)
    {
        return self->comms[self->active_comm_id];
    }

    return NULL;
}
//authored by Nathan
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
//authored by Nathan
void CommandHandler_set_on_heartbeat_lost(struct CommandHandler *self, uint32_t heartbeat_margin, CommandHandlerHeartbeatLostFunction handler)
{
    if (self)
    {
        self->heartbeat_margin = heartbeat_margin;
        self->on_heartbeat_lost = handler;
    }
}
//authored by Nathan
void CommandHandler_destroy(struct CommandHandler *self)
{
    if (self)
    {
        // Also empty queues...
        free(self);
    }
}