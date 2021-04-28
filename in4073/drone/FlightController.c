//
// Created by nathan on 26-04-21.
//

#include "FlightController.h"
#include <stdlib.h>
#include <string.h>

void FlightController_loop(void *context, int delta_ms)
{
    struct FlightController *fc = (struct FlightController *)context;

    switch (fc->mode) {
        case FC_MODE_INIT:

            break;
        case FC_MODE_SAFE:

            break;
        case FC_MODE_PANIC:

            break;
        case FC_MODE_MANUAL:

            break;
        case FC_MODE_CALIBRATE:

            break;
        case FC_MODE_YAW:

            break;
        case FC_MODE_FULL:

            break;
        case FC_MODE_RAW:

            break;
        case FC_MODE_HOLD_HEIGHT:

            break;
        case FC_MODE_WIRELESS:

            break;
    }
}

struct FlightController FlightController_init(struct IMU *imu, struct Rotor **rotors, uint8_t num_rotors)
{
    struct FlightController fc = {
            .imu = imu,
            .num_rotors = num_rotors,
            .loop = {
                .func = FlightController_loop
            }
    };

    fc.rotors = malloc(num_rotors * sizeof(Rotor *));
    memcpy(&fc.rotors, rotors, num_rotors * sizeof(Rotor *));
}