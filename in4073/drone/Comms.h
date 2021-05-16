//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMS_H
#define QUADCOPTER_FCB_COMMS_H

#include <stdbool.h>
#include "Command.h"
#include "LoopHandler.h"

#define COMMS_QUEUE_LEN 100

struct Comms {
    struct Command *commands[COMMS_QUEUE_LEN];
    struct LoopHandlerControlBlock loop;

    uint16_t first;
    uint16_t last;
    uint16_t count;
};

void Comms_enqueue_command(struct Comms* self, struct Command *command);
struct Command *Comms_dequeue_command(struct Comms* self);
bool Comms_available(struct Comms* self);

#endif //QUADCOPTER_FCB_COMMS_H
