//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_ROTOR_H
#define QUADCOPTER_FCB_ROTOR_H

#include <stdint.h>
#include "LoopHandler.h"

struct RotorMap
{
    uint16_t rpm_min;
    uint16_t rpm_max;
    // ... whatever else
};

void Rotor_loop(void *context, int delta_ms);

struct Rotor
{
    int x_offset;
    int y_offset;
    uint8_t gpio_num;
    struct RotorMap *r_map;
    struct LoopHandler_cb loop;

    uint16_t target_rpm;
};

struct RotorMap RotorMap_init(uint16_t rpm_min, uint16_t rpm_max);

struct Rotor Rotor_init(struct RotorMap *map, int x_offset, int y_offset, uint8_t gpio_num);

void Rotor_set_rpm(struct Rotor *self, uint16_t rpm);
void Rotor_set_thrust(struct Rotor *self, uint16_t thrust);


#endif //QUADCOPTER_FCB_ROTOR_H
