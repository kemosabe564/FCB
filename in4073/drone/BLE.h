//
// Created by nathan on 28-04-21.
//

#ifndef QUADCOPTER_FCB_BLE_H
#define QUADCOPTER_FCB_BLE_H

#include "Comms.h"

struct Comms *BLE_create();

void BLE_loop(void *context, uint32_t delta_us);

void BLE_destroy(struct Comms *self);

#endif //QUADCOPTER_FCB_BLE_H
