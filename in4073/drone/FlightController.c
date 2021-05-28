//
// Created by nathan on 26-04-21.
//

#include "FlightController.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../mpu6050/inv_mpu_dmp_motion_driver.h"


#include "../control.h"

#include "../hal/adc.h"
#include "../mpu6050/mpu6050.h"
#include "../hal/timers.h"



void FlightController_loop(void *context, uint32_t delta_us)
{
    struct FlightController *self = (struct FlightController *)context;
    self->current_psi = psi;
    self->current_phi = phi;
    self->current_theta = theta;

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
            uint32_t now = get_time_us();

            if (self->input_ts != 0 && (now - self->input_ts > 50000))
            {
                FlightController_change_mode(self,Panic);
            }

            uint16_t t = FlightController_map_throttle(self);

            uint16_t rpm0 = SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(FlightController_set_limited_rpm(t + self->pitch_angle - self->yaw_rate))];
            uint16_t rpm1 = SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(FlightController_set_limited_rpm(t + self->roll_angle + self->yaw_rate))];
            uint16_t rpm2 = SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(FlightController_set_limited_rpm(t - self->pitch_angle - self->yaw_rate))];
            uint16_t rpm3 = SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(FlightController_set_limited_rpm(t - self->pitch_angle + self->yaw_rate))];


            Rotor_set_rpm(self->rotors[0], rpm0);
            Rotor_set_rpm(self->rotors[1], rpm1);
            Rotor_set_rpm(self->rotors[2], rpm2);
            Rotor_set_rpm(self->rotors[3], rpm3);
        }
            break;
        case Calibrate:
            //dmp_enable_gyro_cal(1); //not required

            //turning down all the motors
            for (int i =0; i <self->num_rotors; i++)
            {
                Rotor_set_rpm(self->rotors[i],0);
            }

            get_sensor_data();
            //If you dont actually need to check ,just get the values here and wait manually
            if (self->is_calibrating)
            {
                CommandHandler_send_command(self->ch, Command_make_debug_msg("Cal Start\n"));
                self->calibrate_start_time=get_time_us();
                //TODO:This is terrible but anyway
                while(get_time_us() < (self->calibrate_start_time + CALIBRATION_WAIT_TIME_US));
                get_sensor_data();
                self->theta_offset = theta;
                self->phi_offset = phi;
                CommandHandler_send_command(self->ch, Command_make_debug_msg("Cal End\n"));
                self->is_calibrating=false;
            }


            break;
        case Yaw: {
            get_sensor_data();
            int16_t t = FlightController_map_throttle(self);
            //get set point
            int16_t setPoint = self->yaw_rate;
            //get sensor reading
            //TODO:Divide by time
            int16_t  psi_rate = (self-> current_psi - self->previous_psi );
            //CommandHandler_send_command(self->ch, Command_make_debug_msg("sr %d\n",sr));
//            CommandHandler_send_command(self->ch, Command_make_debug_msg("Psi old %d\n",self->previous_psi));
//            CommandHandler_send_command(self->ch, Command_make_debug_msg("Psi new %d\n",self->current_psi));
//            CommandHandler_send_command(self->ch, Command_make_debug_msg("dPsi %d\n",psi_rate));
            //calculate error
            int16_t yaw_error = setPoint - psi_rate;
            //calculate compensation and apply
            int16_t yaw_compensation = YAW_P * yaw_error;

            Rotor_set_rpm(self->rotors[0], FlightController_set_limited_rpm(t + self->pitch_angle - yaw_compensation));
            Rotor_set_rpm(self->rotors[1], FlightController_set_limited_rpm(t + self->roll_angle  + yaw_compensation));
            Rotor_set_rpm(self->rotors[2], FlightController_set_limited_rpm(t - self->pitch_angle - yaw_compensation));
            Rotor_set_rpm(self->rotors[3], FlightController_set_limited_rpm(t - self->roll_angle  + yaw_compensation));
        }
            break;
        case Full:
            get_sensor_data();
            //get throttle 0-255
            int16_t t = FlightController_map_throttle(self);
            //get set points 0 - 255
            int16_t phi_setPoint = FlightController_roll_over_angle(self->phi_offset/256 + self->roll_angle);
            int16_t theta_setPoint = FlightController_roll_over_angle(self->theta_offset/256 + self->pitch_angle);
            int16_t  yaw_setPoint = self->yaw_rate;
            //calculate rate of change
            int16_t  psi_rate = (self-> current_psi - self->previous_psi );
            int16_t  phi_rate = (self-> current_phi - self->previous_phi );
            int16_t  theta_rate = (self-> current_theta - self->previous_theta );

            //calculate error1
            //TODO: need to make sure this can go the other way around also
            int16_t yaw_error = yaw_setPoint - psi_rate;
            int16_t roll_error = phi_setPoint - phi/256;
            int16_t  pitch_error = theta_setPoint - theta/256;

            //calculate compensation 1
            int16_t yaw_compensation = YAW_P * yaw_error;
            int16_t roll_rate_setPoint = FULL_P1 * roll_error;
            int16_t pitch_rate_setPoint = FULL_P1 * pitch_error;

            //roll and pitch rate error
            int16_t roll_rate_error = roll_rate_setPoint - phi_rate;
            int16_t pitch_rate_error = pitch_rate_setPoint - pitch_rate;

            //calculate compensation 2
            int16_t roll_rate_compensation = FULL_P2 * roll_rate_error;
            int16_t pitch_rate_compensation = FULL_P2 * pitch_rate_error;

            Rotor_set_rpm(self->rotors[0], FlightController_set_limited_rpm(t + pitch_rate_compensation - yaw_compensation));
            Rotor_set_rpm(self->rotors[1], FlightController_set_limited_rpm(t + roll_rate_compensation  + yaw_compensation));
            Rotor_set_rpm(self->rotors[2], FlightController_set_limited_rpm(t - pitch_rate_compensation - yaw_compensation));
            Rotor_set_rpm(self->rotors[3], FlightController_set_limited_rpm(t - roll_rate_compensation  + yaw_compensation));

            break;
        case Raw:

            break;
        case HoldHeight:

            break;
    }
    self->previous_psi = self->current_psi;
    self->previous_phi = self->current_phi;
    self-> previous_theta = self->current_theta;


}

struct FlightController *FlightController_create(struct IMU *imu, struct Rotor *rotors[], uint8_t num_rotors, struct CommandHandler *ch)
{
    struct FlightController *result = (struct FlightController *)malloc(sizeof(struct FlightController));

    if (result)
    {
        adc_init();
        get_sensor_data();

        result->imu = imu;
        result->loop = LoopHandler_init_controlblock(FlightController_loop);
        result->mode = Init;
        result->debug_mode = false;
        result->on_changed_mode = NULL;
        result->num_rotors = num_rotors;
        result->input_ts = 0;
        result->rotors = (struct Rotor **)malloc(num_rotors * sizeof(struct Rotor *));
        memcpy(result->rotors, rotors, num_rotors * sizeof(struct Rotor *));
        result->current_psi = psi;
        result->previous_psi = psi;
        result->current_phi=phi;
        result->previous_phi=phi;
        result->current_theta=theta;
        result->previous_theta=theta;
        result->ch =ch;
        result->phi_offset = phi;
        result->theta_offset=theta;
        result->is_calibrating=false;
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

                // might not be necessary
                self->input_ts = 0;

                self->mode = mode;
                return true;
            }
        }
        if(self-> != mode && mode == Calibrate)
        {
            self->is_calibrating=true;
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
        self->pitch_angle = pitch_rate - 127;
        self->roll_angle = roll_rate - 127;
        self->throttle = throttle;

        self->input_ts = get_time_us();
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

uint16_t FlightController_map_throttle(struct  FlightController *self)
{
    uint16_t t;
    if (self->throttle >0)
    {
        t= (self->throttle + 80) * 2;
    }
    else
    {
        t=0;
    }
    return t;
}

uint16_t FlightController_set_limited_rpm(uint16_t rpm)
{
    if (rpm < MINIMUM_RPM)
    {
        rpm = MINIMUM_RPM;
    }
    return rpm;
}

int16_t FlightController_roll_over_angle(int16_t angle)
{
    if(angle > 127)
    {
        angle = -127 + angle - 127;
    }
    if (angle<-127)
    {
        angle = 127 + angle + 127;
    }
    return angle;
}

uint16_t FlightController_sqrt_index_bounds(uint16_t rpm_in)
{
    if (rpm_in>999){
        rpm_in = 999;
    }
    if (rpm_in < 0){
        rpm_in = 0 ;
    }
    return rpm_in;
}
