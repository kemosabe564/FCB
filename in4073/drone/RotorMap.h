//
// Created by nathan on 28-04-21.
//

#ifndef QUADCOPTER_FCB_ROTORMAP_H
#define QUADCOPTER_FCB_ROTORMAP_H

#include <stdint.h>

struct RotorMap
{
    uint16_t rpm_min;
    uint16_t rpm_max;
    uint16_t thrust_min;
    uint16_t thrust_max;
};

struct RotorMap *RotorMap_create(uint16_t rpm_min, uint16_t rpm_max);
void RotorMap_destroy(struct RotorMap *self);

#endif //QUADCOPTER_FCB_ROTORMAP_H
