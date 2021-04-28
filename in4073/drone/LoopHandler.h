//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_LOOPHANDLER_H
#define QUADCOPTER_FCB_LOOPHANDLER_H


#define LH_LINK(comp) ptr->loop, (void *)&ptr

struct LoopHandler_cb
{
    void (*func)(void *context, int delta_ms);
};

struct LoopHandler
{

};

struct LoopHandler LoopHandler_init();

void LoopHandler_loop(struct LoopHandler *self, struct LoopHandler_cb *cb, void *context, int refresh_rate);

#endif //QUADCOPTER_FCB_LOOPHANDLER_H
