//
// Created by nathan on 26-04-21.
//

#include "Rotor.h"

void Rotor_loop(void *context, int delta_ms)
{
    Rotor *rotor = (Rotor *)context;


}

struct RotorMap RotorMap_init(uint16_t rpm_min, uint16_t rpm_max)
{
    return {
        .rpm_min = rpm_min,
        .rpm_max = rpm_max
    }
}

struct Rotor Rotor_init(struct RotorMap *map, int x_offset, int y_offset, uint8_t gpio_num)
{
    // GPIO INIT

    return {
        .x_offset = x_offset,
        .y_offset = y_offset,
        .gpio_num = gpio_num,
        .loop = {
            .func = Rotor_loop
        };
    }
}