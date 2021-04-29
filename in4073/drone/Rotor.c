//
// Created by nathan on 26-04-21.
//

#include "Rotor.h"

#include <stdlib.h>

void Rotor_loop(void *context, uint32_t delta_us)
{
//    struct Rotor *rotor = (struct Rotor *)context;

}

struct Rotor *Rotor_create(struct RotorMap *map, int x_offset, int y_offset, uint8_t gpio_num)
{
    // GPIO INIT

    struct Rotor *result = (struct Rotor *)malloc(sizeof(struct Rotor));

    if (result)
    {
        result->x_offset = x_offset;
        result->y_offset = y_offset;
        result->gpio_num = gpio_num;
        result->loop = LoopHandler_init_controlblock(Rotor_loop);
    }

    return result;
}

void Rotor_destroy(struct Rotor *self)
{
    if (self)
    {
        free(self);
    }
}