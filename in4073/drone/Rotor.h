//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_ROTOR_H
#define QUADCOPTER_FCB_ROTOR_H

#include <stdint.h>
#include "LoopHandler.h"

#define SAFE_RPM_LIMIT 800


struct Rotor
{
    int x_offset;
    int y_offset;
    uint8_t motor_idx;
    struct RotorMap *r_map;
    struct LoopHandlerControlBlock loop;

    uint16_t actual_rpm;
    uint16_t target_rpm;
};

void Rotor_loop(void *context, uint32_t delta_us);

struct Rotor *Rotor_create(struct RotorMap *map, uint8_t motor_idx, int x_offset, int y_offset);
void Rotor_set_rpm(struct Rotor *self, uint16_t rpm);
void Rotor_set_thrust(struct Rotor *self, uint16_t thrust);
void Rotor_destroy(struct Rotor *self);

#endif //QUADCOPTER_FCB_ROTOR_H
