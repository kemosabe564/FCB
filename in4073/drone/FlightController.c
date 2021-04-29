//
// Created by nathan on 26-04-21.
//

#include "FlightController.h"
#include <stdlib.h>
#include <string.h>

void FlightController_loop(void *context, uint32_t delta_us)
{
    struct FlightController *fc = (struct FlightController *)context;

    switch (fc->mode) {
        case Init:

            break;
        case Safe:

            break;
        case Panic:

            break;
        case Manual:

            break;
        case Calibrate:

            break;
        case Yaw:

            break;
        case Full:

            break;
        case Raw:

            break;
        case HoldHeight:

            break;
    }
}

struct FlightController *FlightController_create(struct IMU *imu, struct Rotor **rotors, uint8_t num_rotors)
{
    struct FlightController *result = (struct FlightController *)malloc(sizeof(struct FlightController));

    if (result)
    {
        result->imu = imu;
        result->loop = LoopHandler_init_controlblock(FlightController_loop);

        result->rotors = (struct Rotor **)malloc(num_rotors * sizeof(struct Rotor *));
        memcpy(result->rotors, rotors, num_rotors * sizeof(struct Rotor *));
    }

    return result;
}

void FlightController_destroy(struct FlightController *self)
{
    if (self)
    {
        free(self);
    }
}
