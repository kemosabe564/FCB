//
// Created by nathan on 02-06-21.
//

#ifndef QUADCOPTER_FCB_TELEMETRY_H
#define QUADCOPTER_FCB_TELEMETRY_H

#include "LoopHandler.h"
#include <stdbool.h>

struct Telemetry
{
    struct Rotor **rotors;
    uint8_t num_rotors;
    struct IMU *imu;
    struct CommandHandler *ch;

    bool enabled;
    struct LoopHandlerControlBlock loop;
};


struct Telemetry *Telemetry_create(struct CommandHandler *ch, struct IMU *imu, struct Rotor *rotors[], uint8_t num_rotors);
void Telemetry_loop(void *context, uint32_t delta_us);
void Telemetry_set_enabled(struct Telemetry *self, bool enabled);
void Telemetry_destroy(struct Telemetry *self);

#endif //QUADCOPTER_FCB_TELEMETRY_H
