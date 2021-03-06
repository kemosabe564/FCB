//
// Created by nathan on 02-06-21.
//

#include "Telemetry.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Command.h"
#include "CommandHandler.h"
#include "IMU.h"
#include "Rotor.h"
#include "Debug.h"


#include "../hal/gpio.h"
#include "nrf_gpio.h"
//authored by Nathan
struct Telemetry *Telemetry_create(struct CommandHandler *ch, struct IMU *imu, struct Rotor *rotors[], uint8_t num_rotors)
{
    struct Telemetry *result = (struct Telemetry *)malloc(sizeof(struct Telemetry));

    if (result)
    {
        result->ch = ch;
        result->imu = imu;
        result->num_rotors = num_rotors;
        result->rotors = (struct Rotor **)malloc(num_rotors * sizeof(struct Rotor *));
        memcpy(result->rotors, rotors, num_rotors * sizeof(struct Rotor *));

        result->loop = LoopHandler_init_controlblock(Telemetry_loop);
        result->enabled = true;
    }

    return result;
}
//authored by Nathan
void Telemetry_set_enabled(struct Telemetry *self, bool enabled)
{
    if (self)
    {
        self->enabled = enabled;
    }
}
//authored by Nathan
void Telemetry_loop(void *context, uint32_t delta_us)
{
    struct Telemetry *self = (struct Telemetry *)context;
    //DEBUG(0,"b %d",self->imu->battery_average);

    if (self->enabled)
    {
        struct Command *cmd = Command_make_telemetry(
            self->imu->roll_angle,
            self->imu->pitch_angle,
            self->imu->yaw_rate,
            self->rotors[0]->actual_rpm,
            self->rotors[1]->actual_rpm,
            self->rotors[2]->actual_rpm,
            self->rotors[3]->actual_rpm
        );

        CommandHandler_send_command(self->ch, cmd);
    }

    nrf_gpio_pin_toggle(BLUE);
}
//authored by Nathan
void Telemetry_destroy(struct Telemetry *self)
{
    if (self)
    {
        free(self->rotors);
        free(self);
    }
}