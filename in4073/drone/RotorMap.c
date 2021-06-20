//
// Created by nathan on 28-04-21.
//

#include "RotorMap.h"

#include <stdlib.h>
//authored by Nathan
struct RotorMap *RotorMap_create(uint16_t rpm_min, uint16_t rpm_max)
{
    struct RotorMap *result = (struct RotorMap *)malloc(sizeof (struct RotorMap));

    if (result)
    {
        result->rpm_max = rpm_max;
        result->rpm_min = rpm_min;
    }

    return result;
}
//authored by Nathan
void RotorMap_destroy(struct RotorMap *self)
{
    if (self)
    {
        free(self);
    }
}