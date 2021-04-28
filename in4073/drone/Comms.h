//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMS_H
#define QUADCOPTER_FCB_COMMS_H

#include "Command.h"
#include "../utils/queue.h"

#define COMMS_QUEUE_LEN 100

struct Comms {
    struct Command commands[COMMS_QUEUE_LEN];
    Queue *comm_queue;
};

void Comms_init(struct *Comms self, Queue *queue);

#endif //QUADCOPTER_FCB_COMMS_H
