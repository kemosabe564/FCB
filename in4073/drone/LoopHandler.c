//
// Created by nathan on 26-04-21.
//

#include "LoopHandler.h"

#include <stdlib.h>

#include "../hal/timers.h"
#include "../mpu6050/mpu6050.h"

struct LoopHandler *LoopHandler_create()
{
    struct LoopHandler *result = (struct LoopHandler *)malloc(sizeof(struct LoopHandler));

    if (result)
    {
        timers_init();
    }

    return result;
}

struct LoopHandlerControlBlock LoopHandler_init_controlblock(LoopHandlerFunction function)
{
    struct LoopHandlerControlBlock cb = {
        .func = function,
        .next_release = 0,
        .last_activation = 0,
    };

    return cb;
}

void LoopHandler_loop(struct LoopHandler *self, struct LoopHandlerControlBlock *cb, void *context, uint32_t period_us) {
    uint32_t now = get_time_us();

    if (period_us == 0) {
        cb->func(context, 0);
    } else
    {

        if (cb->next_release == 0)
        {
            cb->func(context, 0);
            cb->next_release = now + period_us;
            cb->last_activation = now;
        }
        else if(cb->next_release <= now)
        {
            cb->func(context, now - cb->last_activation);
            cb->next_release += period_us;
            cb->last_activation = now;
        }
    }


}

void LoopHandler_destroy(struct LoopHandler *self)
{
    if (self)
    {
        free(self);
    }
}