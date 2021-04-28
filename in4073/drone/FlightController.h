//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_FLIGHTCONTROLLER_H
#define QUADCOPTER_FCB_FLIGHTCONTROLLER_H

#include <stdint.h>
#include "LoopHandler.h"
#include "Rotor.h"
#include "IMU.h"

#define FC_MODE_INIT 1
#define FC_MODE_SAFE 2
#define FC_MODE_PANIC 3
#define FC_MODE_MANUAL 4
#define FC_MODE_CALIBRATE 5
#define FC_MODE_YAW 6
#define FC_MODE_FULL 7
#define FC_MODE_RAW 8
#define FC_MODE_HOLD_HEIGHT 9
#define FC_MODE_WIRELESS 10

struct FlightController
{
    struct Rotor **rotors;
    uint8_t num_rotors;
    struct LoopHandler_cb loop;

    uint8_t mode;
};

struct FlightController FlightController_init(struct IMU *imu, struct Rotor **rotors, uint8_t num_rotors);

void FlightController_loop(void *context, int delta_ms);

#endif //QUADCOPTER_FCB_FLIGHTCONTROLLER_H
