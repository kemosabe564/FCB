//
// Created by nathan on 26-04-21.
//

#include "FlightController.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "../control.h"

#include "../hal/adc.h"
#include "../mpu6050/mpu6050.h"

void FlightController_loop(void *context, uint32_t delta_us)
{
    struct FlightController *self = (struct FlightController *)context;

//    static int check = 0;
//    printf("FlightController_loop %d - Bat %4d - Motor %d - Mode::%s \n", check++, bat_volt, motor[0], FlightControllerMode_to_str(self->mode));
//
//    static int incrementing = 1;

    switch (self->mode) {
        case Init:
            self->debug_mode = (bat_volt == 0);

            FlightController_change_mode(self, Safe);
            break;
        case Safe:
            for (int i =0; i <self->num_rotors; i++)
            {
                Rotor_set_rpm(self->rotors[i],0);
            }

//
//            Rotor_set_rpm(self->rotors[0], self->rotors[0]->actual_rpm + (incrementing ? 10 : -10));
//
//            if (self->rotors[0]->actual_rpm >= 250)
//                incrementing = 0;
//
//            if (self->rotors[0]->actual_rpm <= 50)
//                incrementing = 1;

//            if (FlightController_check_rotors_safe(self))
//                FlightController_change_mode(self,Safe);

//            for (int i = 0; i < self->num_rotors; i++)
//                Rotor_set_rpm(self->rotors[i], 0);

            break;
        case Panic: {
            bool check_if_all_zero = true;
            for (int i = 0; i < self->num_rotors; i++)
            {
                if (self->rotors[i]->actual_rpm > 0)
                {
                    Rotor_set_rpm(self->rotors[i], self->rotors[i]->actual_rpm - 1);
                    check_if_all_zero = false;
                }
            }

            if (check_if_all_zero)
            {
                FlightController_change_mode(self,Safe);
            }
        }
            break;
        case Manual: {
            //TODO:Check if this simulates yawing
//            int base_rpm = 200;
////            int additional_rpm = 100;

            Rotor_set_rpm(self->rotors[0], self->throttle + self->pitch_rate - self->yaw_rate);
            Rotor_set_rpm(self->rotors[1], self->throttle + self->roll_rate  + self->yaw_rate);
            Rotor_set_rpm(self->rotors[2], self->throttle - self->pitch_rate - self->yaw_rate);
            Rotor_set_rpm(self->rotors[3], self->throttle - self->roll_rate  + self->yaw_rate);
        }
            break;
        case Calibrate:

            break;
        case Yaw: {
            //TODO: check if we need to add minus or plus sr
            int p_yaw = 1;
            int compensation_yaw = p_yaw * sr ;
            int base_rpm = 200;
            Rotor_set_rpm(self->rotors[0], base_rpm);
            Rotor_set_rpm(self->rotors[1],base_rpm + compensation_yaw);
            Rotor_set_rpm(self->rotors[2], base_rpm);
            Rotor_set_rpm(self->rotors[3],base_rpm + compensation_yaw);
        }
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
        result->on_changed_mode = NULL;
        result->num_rotors = num_rotors;
        result->rotors = (struct Rotor **)malloc(num_rotors * sizeof(struct Rotor *));
        memcpy(result->rotors, rotors, num_rotors * sizeof(struct Rotor *));
    }

    return result;
}

bool FlightController_change_mode(struct FlightController *self, enum FlightControllerMode mode)
{
    if (self)
    {
        if (self->mode != mode && (FlightController_check_rotors_safe(self) || mode == Panic))
        {
            // We can only change from panic over to safe
            if (self->mode != Panic || (self->mode == Panic && mode == Safe))
            {
                if (self->on_changed_mode)
                {
                    self->on_changed_mode(mode, self->mode);
                }

                self->mode = mode;
                return true;
            }
        }
    }
    return false;
}

void FlightController_set_on_change_mode(struct FlightController *self, FlightControllerChangedMode handler)
{
    if (self)
    {
        self->on_changed_mode = handler;
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
                return false;
            }
        }
    }
    return true;
}


void FlightController_set_throttle(struct FlightController *self, uint16_t throttle)
{
    if (self)
    {
        self->throttle = throttle;
    }
}


void FlightController_set_controls(struct FlightController *self, int16_t yaw_rate, int16_t pitch_rate, int16_t roll_rate, uint16_t throttle)
{
    if (self)
    {
        self->yaw_rate = yaw_rate - 127;
        self->pitch_rate = pitch_rate - 127;
        self->roll_rate = roll_rate - 127;
        self->throttle = throttle + 150;
    }
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
