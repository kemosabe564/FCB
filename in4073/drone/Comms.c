//
// Created by nathan on 26-04-21.
//

#include "Comms.h"

struct Comms Comms_init(Queue *queue)
{
    struct Comms comm;
    comm.comm_queue = queue;

    return comm;
}