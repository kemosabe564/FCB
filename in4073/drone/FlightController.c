//
// Created by nathan on 26-04-21.
//

#include "FlightController.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../hal/adc.h"

void FlightController_loop(void *context, uint32_t delta_us)
{
    struct FlightController *self = (struct FlightController *)context;

    static int check = 0;
    printf("FlightController_loop %d - Bat %4d - Mode::%s \n", check++, bat_volt, FlightControllerMode_to_str(self->mode));

    switch (self->mode) {
        case Init:
            self->debug_mode = (bat_volt == 0);

            FlightController_change_mode(self, Safe);

            Rotor_set_rpm(self->rotors[0], 10);

            break;
        case Safe:

            Rotor_set_rpm(self->rotors[0], self->rotors[0]->actual_rpm - 1);

//            if (FlightController_check_rotors_safe(self))
//                FlightController_change_mode(self,Safe);

//            for (int i = 0; i < self->num_rotors; i++)
//                Rotor_set_rpm(self->rotors[i], 0);

            break;
        case Panic:

//            for (int i = 0; i < self->num_rotors; i++)
//                Rotor_set_rpm(self->rotors[i], self->rotors[i]->actual_rpm - 1);



            break;
        case Manual:

            Rotor_set_rpm(self->rotors[0], 10);

            break;
        case Calibrate:

            break;
        case Yaw:

            break;
        case Full:

            break;
        case Raw:

            break;
        case HoldHeight:

            break;
    }
}

struct FlightController *FlightController_create(struct IMU *imu, struct Rotor *rotors[], uint8_t num_rotors)
{
    struct FlightController *result = (struct FlightController *)malloc(sizeof(struct FlightController));

    if (result)
    {
        adc_init();

        result->imu = imu;
        result->loop = LoopHandler_init_controlblock(FlightController_loop);
        result->mode = Init;
        result->debug_mode = false;
        result->num_rotors = num_rotors;
        result->rotors = (struct Rotor **)malloc(num_rotors * sizeof(struct Rotor *));
        memcpy(result->rotors, rotors, num_rotors * sizeof(struct Rotor *));
    }

    return result;
}

void FlightController_change_mode(struct FlightController *self, enum FlightControllerMode mode)
{
    if (self)
    {
        printf("Attempting to change mode \n");
        if (self->mode != mode && (FlightController_check_rotors_safe(self) || mode == Panic))
        {
            // We can only change from panic over to safe
            if (self->mode != Panic || (self->mode == Panic && mode == Safe))
            {
                printf("Changing mode from %s to %s \n",
                       FlightControllerMode_to_str(self->mode),
                       FlightControllerMode_to_str(mode));

                self->mode = mode;
            }
        }
    }
}

bool FlightController_check_rotors_safe(struct FlightController *self)
{
    if (self)
    {
        for (uint8_t i = 0; i < self->num_rotors; i += 1)
        {
            if (self->rotors[i]->actual_rpm)
            {
                printf("Not safe\n");
                return false;
            }
        }
    }
    printf("Safe\n");
    return true;
}

char *FlightControllerMode_to_str(enum FlightControllerMode mode)
{
    switch (mode) {
        case Init: return "Init";
        case Safe: return "Safe";
        case Panic: return "Panic";
        case Manual: return "Manual";
        case Calibrate: return "Calibrate";
        case Yaw: return "Yaw";
        case Full: return "Full";
        case Raw: return "Raw";
        case HoldHeight: return "HoldHeight";
    }
    return "";
}

void FlightController_destroy(struct FlightController *self)
{
    if (self)
    {
        free(self);
    }
}
