//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_IMU_H
#define QUADCOPTER_FCB_IMU_H

#include "LoopHandler.h"

struct IMU
{
    struct LoopHandlerControlBlock loop;
};

void IMU_loop(void *context, uint32_t delta_us);

struct IMU *IMU_create();
void IMU_destroy(struct IMU *self);

#endif //QUADCOPTER_FCB_IMU_H
