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
    Init = 1,
    Safe = 2,
    Panic = 3,
    Manual = 4,
    Calibrate = 5,
    Yaw = 6,
    Full = 7,
    Raw = 8,
    HoldHeight = 9
};

typedef void (*FlightControllerChangedMode)(enum FlightControllerMode new_mode, enum FlightControllerMode old_mode);

struct FlightController
{
    struct Rotor **rotors;
    uint8_t num_rotors;
    enum FlightControllerMode mode;
    struct IMU *imu;
    bool debug_mode;

    int16_t yaw_rate;
    int16_t pitch_rate;
    int16_t roll_rate;
    uint16_t throttle;

    FlightControllerChangedMode on_changed_mode;

    struct LoopHandlerControlBlock loop;
};

struct FlightController *FlightController_create(struct IMU *imu, struct Rotor *rotors[], uint8_t num_rotors);

char *FlightControllerMode_to_str(enum FlightControllerMode mode);

bool FlightController_change_mode(struct FlightController *self, enum FlightControllerMode mode);
void FlightController_set_on_change_mode(struct FlightController *self, FlightControllerChangedMode handler);
bool FlightController_check_rotors_safe(struct FlightController *self);
void FlightController_set_throttle(struct FlightController *self, uint16_t throttle);
void FlightController_set_controls(struct FlightController *self, int16_t yaw_rate, int16_t pitch_rate, int16_t roll_rate, uint16_t throttle);
void FlightController_loop(void *context, uint32_t delta_us);
void FlightController_destroy(struct FlightController *self);

#endif //QUADCOPTER_FCB_FLIGHTCONTROLLER_H
