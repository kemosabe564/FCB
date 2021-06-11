//
// Created by nathan on 28-04-21.
//

#ifndef QUADCOPTER_FCB_COMMANDHANDLER_H
#define QUADCOPTER_FCB_COMMANDHANDLER_H

#include "Comms.h"
#include "Command.h"
#include "FlightController.h"

//#ifndef
#define COMMANDHANDLER_MAX_COMMS 2
//#endif

typedef void (*CommandHandlerFunction)(struct Command *command);
typedef void (*CommandHandlerHeartbeatLostFunction)();

enum HeartbeatState
{
    Dead = 0,
    Alive = 1,
};

enum CommandHandlerState
{
    CH_Init = 0,
    CH_Running = 1,
};

struct CommandHandler
{
    CommandHandlerFunction handler;
    uint8_t active_comm_id;
    struct LoopHandlerControlBlock loop;
    struct Comms* comms[COMMANDHANDLER_MAX_COMMS];
    enum CommandHandlerState state;

    uint8_t heartbeat_seq;
    uint32_t heartbeat_ts;
    CommandHandlerHeartbeatLostFunction on_heartbeat_lost;
    enum HeartbeatState heartbeat_state;
    uint32_t heartbeat_margin;
};

struct CommandHandler *CommandHandler_create(uint8_t active_comms, CommandHandlerFunction handler);
void CommandHandler_add_comms(struct CommandHandler *self, uint8_t id, struct Comms *comms);
struct Comms *CommandHandler_get_active_comms(struct CommandHandler *self);
void CommandHandler_set_on_heartbeat_lost(struct CommandHandler *self, uint32_t heartbeat_margin, CommandHandlerHeartbeatLostFunction handler);
void __CommandHandler_command_handler_internal(struct CommandHandler *self, struct Command *command);
void CommandHandler_send_command(struct CommandHandler *self, struct Command *command);;
void CommandHandler_loop(void *context, uint32_t delta_us);
void CommandHandler_destroy(struct CommandHandler *self);


#endif //QUADCOPTER_FCB_COMMANDHANDLER_H
