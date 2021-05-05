//
// Created by nathan on 26-04-21.
//

#include "Rotor.h"

#include <stdlib.h>
#include "../control.h"
#include "../hal/timers.h"
#include "../hal/gpio.h"
#include <stdio.h>

void Rotor_loop(void *context, uint32_t delta_us)
{
    struct Rotor *rotor = (struct Rotor *)context;

    rotor->actual_rpm = rotor->target_rpm; // replaced by some control function

    motor[rotor->motor_idx] = rotor->actual_rpm;
}

struct Rotor *Rotor_create(struct RotorMap *map, uint8_t motor_idx, int x_offset, int y_offset)
{
    struct Rotor *result = (struct Rotor *)malloc(sizeof(struct Rotor));

    if (result)
    {
        timers_init();
        gpio_init();

        result->x_offset = x_offset;
        result->y_offset = y_offset;
        result->motor_idx = motor_idx;
        result->actual_rpm = 0;
        result->target_rpm = 0;
        result->loop = LoopHandler_init_controlblock(Rotor_loop);
    }

    return result;
}

void Rotor_set_rpm(struct Rotor *self, uint16_t rpm) {
    if (self)
    {
        self->target_rpm = rpm;
    }
}

void Rotor_set_thrust(struct Rotor *self, uint16_t thrust);

void Rotor_destroy(struct Rotor *self)
{
    if (self)
    {
        free(self);
    }
}