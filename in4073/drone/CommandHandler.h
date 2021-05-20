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

struct CommandHandler
{
    CommandHandlerFunction handler;
    uint8_t active_comm_id;
    struct LoopHandlerControlBlock loop;
    struct Comms* comms[COMMANDHANDLER_MAX_COMMS];
};

struct CommandHandler *CommandHandler_create(uint8_t active_comms, CommandHandlerFunction handler);
void CommandHandler_add_comms(struct CommandHandler *self, uint8_t id, struct Comms *comms);
struct Comms *CommandHandler_get_active_comms(struct CommandHandler *self);
void CommandHandler_loop(void *context, uint32_t delta_us);
void CommandHandler_destroy(struct CommandHandler *self);


#endif //QUADCOPTER_FCB_COMMANDHANDLER_H