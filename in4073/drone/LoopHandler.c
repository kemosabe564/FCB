//
// Created by nathan on 26-04-21.
//

#include "LoopHandler.h"

struct LoopHandler LoopHandler_init()
{
    return struct LoopHandler;
}

void LoopHandler_loop(struct LoopHandler *self, struct LoopHandler_cb *cb, void *context, int refresh_rate)
{
    cb->func(context, 1);
}