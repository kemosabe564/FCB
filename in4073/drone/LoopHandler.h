//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_LOOPHANDLER_H
#define QUADCOPTER_FCB_LOOPHANDLER_H

#include <stdint.h>

#define LH_LINK(ptr) &ptr->loop, (void *)ptr
#define LH_HZ_TO_PERIOD(hz) (1000000 / hz)
#define LH_MS_TO_US(ms) (ms * 1000)

typedef void (*LoopHandlerFunction)(void *context, uint32_t delta_us);

struct LoopHandlerControlBlock
{
    uint32_t next_release; // next release in us
    uint32_t last_activation;
    LoopHandlerFunction func;
};

struct LoopHandler
{

};

struct LoopHandler *LoopHandler_create();
struct LoopHandlerControlBlock LoopHandler_init_controlblock(LoopHandlerFunction function);
void LoopHandler_loop(struct LoopHandler *self, struct LoopHandlerControlBlock *cb, void *context, uint32_t period_us);
void LoopHandler_destroy(struct LoopHandler *self);

#endif //QUADCOPTER_FCB_LOOPHANDLER_H
