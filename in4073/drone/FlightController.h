//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_FLIGHTCONTROLLER_H
#define QUADCOPTER_FCB_FLIGHTCONTROLLER_H

#include <stdint.h>
#include "LoopHandler.h"
#include "Rotor.h"
#include "IMU.h"

enum FlightControllerMode
{
    Init,
    Safe,
    Panic,
    Manual,
    Calibrate,
    Yaw,
    Full,
    Raw,
    HoldHeight
};

struct FlightController
{
    struct Rotor **rotors;
    uint8_t num_rotors;
    enum FlightControllerMode mode;
    struct IMU *imu;

    struct LoopHandlerControlBlock loop;
};

struct FlightController *FlightController_create(struct IMU *imu, struct Rotor **rotors, uint8_t num_rotors);
void FlightController_loop(void *context, uint32_t delta_us);
void FlightController_destroy(struct FlightController *self);

#endif //QUADCOPTER_FCB_FLIGHTCONTROLLER_H
