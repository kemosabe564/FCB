//
// Created by nathan on 28-04-21.
//

#ifndef QUADCOPTER_FCB_SERIAL_H_H
#define QUADCOPTER_FCB_SERIAL_H_H

#include "Comms.h"

struct Comms *Serial_create(int32_t baud_rate);

void Serial_loop(void *context, uint32_t delta_us);

void Serial_destroy(struct Comms *self);

#endif //QUADCOPTER_FCB_SERIAL_H_H
