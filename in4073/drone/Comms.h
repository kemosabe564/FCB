//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMS_H
#define QUADCOPTER_FCB_COMMS_H

#include <stdbool.h>
#include "Command.h"
#include "LoopHandler.h"
#include "CommandQueue.h"

struct Comms {
    struct LoopHandlerControlBlock loop;

    struct CommandQueue send_queue;
    struct CommandQueue receive_queue;
};

#endif //QUADCOPTER_FCB_COMMS_H
