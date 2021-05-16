//
// Created by nathan on 26-04-21.
//

#include "IMU.h"

#include <stdlib.h>

struct IMU *IMU_create()
{
    struct IMU *result = (struct IMU *)malloc(sizeof(struct IMU));

    if (result)
    {
        result->loop = LoopHandler_init_controlblock(IMU_loop);
    }

    return result;
}

void IMU_loop(void *context, uint32_t delta_us)
{
//    struct IMU *imu = (struct IMU *)context;


}

void IMU_destroy(struct IMU *self)
{
    if (self)
    {
        free(self);
    }
}