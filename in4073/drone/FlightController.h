//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_FLIGHTCONTROLLER_H
#define QUADCOPTER_FCB_FLIGHTCONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "LoopHandler.h"
#include "Rotor.h"
#include "IMU.h"

enum FlightControllerMode
{
    Init = 0,
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
    bool debug_mode;

    struct LoopHandlerControlBlock loop;
};

struct FlightController *FlightController_create(struct IMU *imu, struct Rotor *rotors[], uint8_t num_rotors);

char *FlightControllerMode_to_str(enum FlightControllerMode mode);

void FlightController_change_mode(struct FlightController *self, enum FlightControllerMode mode);
bool FlightController_check_rotors_safe(struct FlightController *self);
void FlightController_loop(void *context, uint32_t delta_us);
void FlightController_destroy(struct FlightController *self);

#endif //QUADCOPTER_FCB_FLIGHTCONTROLLER_H
