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

#include "../mpu6050/mpu6050.h"
#include "../hal/timers.h"

#include "IMU.h"
#include "Debug.h"
#include "Rotor.h"


void FlightController_loop(void *context, uint32_t delta_us)
{
    struct FlightController *self = (struct FlightController *)context;

    self->current_psi = self->imu->yaw_rate;
    self->current_phi = self->imu->roll_angle;
    self->current_theta = self->imu->pitch_angle;

//    static int check = 0;
//    printf("FlightController_loop %d - Bat %4d - Motor %d - Mode::%s \n", check++, bat_volt, motor[0], FlightControllerMode_to_str(self->mode));
//
//    static int incrementing = 1;

      //comment this for debugging
//    if (self->imu->battery_voltage < BAT_THRESHOLD && self->mode != Safe && self->mode != Panic)
//    {
//        FlightController_change_mode(self,Panic);
//        DEBUG(0, "BATTERY LOW: %d", bat_volt);
//    }

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
            if (self->input_ts != 0 && (now > self->input_ts) && ((now - self->input_ts) > 50000))
            {
                DEBUG(0, "HB:%d, %d, %d", now, self->input_ts, (now - self->input_ts));
                FlightController_change_mode(self,Panic);
            }

            uint16_t t = FlightController_map_proportional(self);

            //DEBUG(0, "Mapped Throttle %d",t);

            if (t<1)
            {
                for (int i =0; i <self->num_rotors; i++)
                {
                    Rotor_set_rpm(self->rotors[i],0);
                }
            }
            else{
                uint16_t rpm0 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + self->pitch_angle - self->yaw_rate)]);
                uint16_t rpm1 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + self->roll_angle + self->yaw_rate)]);
                uint16_t rpm2 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - self->pitch_angle - self->yaw_rate)]);
                uint16_t rpm3 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - self->roll_angle + self->yaw_rate)]);

                //CommandHandler_send_command(self->ch, Command_make_debug_msg("rpm0 %d\n",rpm0));

                Rotor_set_rpm(self->rotors[0], rpm0);
                Rotor_set_rpm(self->rotors[1], rpm1);
                Rotor_set_rpm(self->rotors[2], rpm2);
                Rotor_set_rpm(self->rotors[3], rpm3);
                //DEBUG(0, "MOTORS %d %d %d %d", rpm0 ,rpm1,rpm2,rpm3);

            }
    
        }
            break;
        case Calibrate:
            //turning down all the motors
            for (int i =0; i <self->num_rotors; i++)
            {
                Rotor_set_rpm(self->rotors[i],0);
            }

            if (self->imu->calibrated)
            {
                DEBUG(0, "Calibrated\n");
                DEBUG(0, "Rolloffset: %d, Pitchoffset: %d", self->imu->roll_angle_offset, self->imu->pitch_angle_offset);

                FlightController_change_mode(self, Safe);
            }
            break;
        case Yaw: {
            uint32_t now = get_time_us();
            if (self->input_ts != 0 && (now > self->input_ts) && ((now - self->input_ts) > 50000))
            {
                DEBUG(0, "HB:%d, %d, %d", now, self->input_ts, (now - self->input_ts));
                FlightController_change_mode(self,Panic);
            }

            int16_t t = FlightController_map_proportional(self);
            //get set point
            int16_t setPoint = self->yaw_rate;
            //get sensor reading
            //int16_t  psi_rate = (self->current_psi - self->previous_psi);
            //int16_t  psi_rate = (self->imu->measured_r);
            int16_t psi_rate = self->imu->imu_psi_rate;
            //calculate error
            int16_t yaw_error = setPoint - psi_rate;
            //calculate compensation and apply
            int16_t yaw_compensation = (self->P * yaw_error) / 10;

            if (t<1)
            {
                for (int i =0; i <self->num_rotors; i++)
                {
                    Rotor_set_rpm(self->rotors[i],0);
                }
            }

            else
            {
                uint16_t rpm0 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + self->pitch_angle - yaw_compensation)]);
                uint16_t rpm1 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + self->roll_angle + yaw_compensation)]);
                uint16_t rpm2 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - self->pitch_angle - yaw_compensation)]);
                uint16_t rpm3 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - self->roll_angle + yaw_compensation)]);

                Rotor_set_rpm(self->rotors[0], rpm0);
                Rotor_set_rpm(self->rotors[1], rpm1);
                Rotor_set_rpm(self->rotors[2], rpm2);
                Rotor_set_rpm(self->rotors[3], rpm3);

                //DEBUG(0, "%d,%d,%d",t,psi_rate, yaw_compensation);
            }

        }
            break;
        case Full:
        {
            uint32_t now = get_time_us();
            if (self->input_ts != 0 && (now > self->input_ts) && ((now - self->input_ts) > 50000))
            {
                DEBUG(0, "HB:%d, %d, %d", now, self->input_ts, (now - self->input_ts));
                FlightController_change_mode(self,Panic);
            }

            //DEBUG(0,"height avg %d",self->imu->barometer_average);

            //get throttle
            int16_t t = FlightController_map_proportional(self);
            //saving this in case we go into hold height
            //self->hold_throttle_raw = self->throttle;
            self->hold_throttle =t;
            //get set points
            int16_t phi_setPoint = (self->roll_angle)/4;
            int16_t theta_setPoint = (self->pitch_angle)/4;
            int16_t  yaw_setPoint = self->yaw_rate;
            //calculate rate of change
//            int16_t  psi_rate = (self-> current_psi - self->previous_psi );
//            int16_t  phi_rate = (self-> current_phi - self->previous_phi );
//            int16_t  theta_rate = (self-> current_theta - self->previous_theta );

            int16_t  psi_rate = self->imu->imu_psi_rate;
            int16_t  phi_rate = self->imu->imu_phi_rate/5;
            int16_t  theta_rate = self->imu->imu_theta_rate/5;

//            int16_t  psi_rate = self->imu->measured_r;
//            int16_t  phi_rate = self->imu->measured_p;
//            int16_t  theta_rate = self->imu->measured_q;

            //calculate error1
            int16_t yaw_error = yaw_setPoint - psi_rate;
            int16_t roll_error = phi_setPoint - FlightController_roll_over_angle((self->imu->roll_angle - self->imu->roll_angle_offset)/ 256);
            int16_t  pitch_error = theta_setPoint - FlightController_roll_over_angle((self->imu->pitch_angle - self->imu->pitch_angle_offset)/ 256);

            //calculate compensation 1
            int16_t yaw_compensation = (self->P * yaw_error) / 10;
            int16_t roll_rate_setPoint = (self->P1 * roll_error) / 10;
            int16_t pitch_rate_setPoint = (self->P1 * pitch_error) / 10;

            //roll and pitch rate error
            int16_t roll_rate_error = roll_rate_setPoint - phi_rate;
            int16_t pitch_rate_error = pitch_rate_setPoint - theta_rate;

            //calculate compensation 2
            int16_t roll_rate_compensation = -(self->P2 * roll_rate_error) / 10;
            int16_t pitch_rate_compensation = -(self->P2 * pitch_rate_error) / 10;
            //pitch_rate_compensation = 0;//-(self->P2 * pitch_rate_error) / 10;


            if (t<1)
            {
                for (int i =0; i <self->num_rotors; i++)
                {
                    Rotor_set_rpm(self->rotors[i],0);
                }
            }

            else
            {
                uint16_t rpm0 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + pitch_rate_compensation - yaw_compensation)]);
                uint16_t rpm1 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + roll_rate_compensation + yaw_compensation)]);
                uint16_t rpm2 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - pitch_rate_compensation- yaw_compensation)]);
                uint16_t rpm3 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - roll_rate_compensation + yaw_compensation)]);

                Rotor_set_rpm(self->rotors[0], rpm0);
                Rotor_set_rpm(self->rotors[1], rpm1);
                Rotor_set_rpm(self->rotors[2], rpm2);
                Rotor_set_rpm(self->rotors[3], rpm3);

//                DEBUG(0, "%d,%d,%d", t, self->P1, self->P2);
                //DEBUG(0, "%d,%d,%d", phi_setPoint, roll_rate_error, roll_rate_compensation);
            }
        }
            break;
        case Raw:

            break;
        case HoldHeight:
        {
            //TODO:write all this in functions to reuse code

            //get set points
            int16_t phi_setPoint = (self->roll_angle)/4;
            int16_t theta_setPoint = (self->pitch_angle)/4;
            int16_t  yaw_setPoint = self->yaw_rate;

            //calculate rate of change
            int16_t  psi_rate = self->imu->imu_psi_rate;
            int16_t  phi_rate = self->imu->imu_phi_rate/5;
            int16_t  theta_rate = self->imu->imu_theta_rate/5;

            //calculate error1
            int16_t yaw_error = yaw_setPoint - psi_rate;
            int16_t roll_error = phi_setPoint - FlightController_roll_over_angle((self->imu->roll_angle - self->imu->roll_angle_offset)/ 256);
            int16_t  pitch_error = theta_setPoint - FlightController_roll_over_angle((self->imu->pitch_angle - self->imu->pitch_angle_offset)/ 256);

            //calculate compensation 1
            int16_t yaw_compensation = (self->P * yaw_error) / 10;
            int16_t roll_rate_setPoint = (self->P1 * roll_error) / 10;
            int16_t pitch_rate_setPoint = (self->P1 * pitch_error) / 10;

            //roll and pitch rate error
            int16_t roll_rate_error = roll_rate_setPoint - phi_rate;
            int16_t pitch_rate_error = pitch_rate_setPoint - theta_rate;

            //calculate compensation 2
            int16_t roll_rate_compensation = -(self->P2 * roll_rate_error) / 10;
            int16_t pitch_rate_compensation = -(self->P2 * pitch_rate_error) / 10;

//            if((self->throttle - self->hold_throttle_raw) != 0)
//            {
//                DEBUG(0,"Throttle touched");
//                FlightController_change_mode(self,Full);
//            }

            //get height rate
            //calculated in self->imu->imu_height_rate

            //error is 0 - rate
            int16_t height_error = 0 - self->imu->imu_height_rate;
            int16_t lift_compensation = (self->H/10) * height_error;

            //increase or decrease lift
            uint16_t rpm0 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(self->hold_throttle + lift_compensation + pitch_rate_compensation - yaw_compensation)]);
            uint16_t rpm1 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(self->hold_throttle + lift_compensation + roll_rate_compensation + yaw_compensation)]);
            uint16_t rpm2 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(self->hold_throttle + lift_compensation - pitch_rate_compensation- yaw_compensation)]);
            uint16_t rpm3 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(self->hold_throttle + lift_compensation - roll_rate_compensation + yaw_compensation)]);

            Rotor_set_rpm(self->rotors[0], rpm0);
            Rotor_set_rpm(self->rotors[1], rpm1);
            Rotor_set_rpm(self->rotors[2], rpm2);
            Rotor_set_rpm(self->rotors[3], rpm3);

        }
            break;
    }

    self->previous_psi = self->current_psi;
    self->previous_phi = self->current_phi;
    self-> previous_theta = self->current_theta;

}

void __FlightController_on_changed_mode(struct FlightController *self, enum FlightControllerMode new_mode, enum FlightControllerMode old_mode)
{
    switch (new_mode) {
        case Safe:
        case Panic: {
            self->yaw_rate = 0;
            self->pitch_angle = 0;
            self->roll_angle = 0;
            if (new_mode == Panic)
            {
                DEBUG(0, "PANIC");
            }
        }
            break;
        case Calibrate:
            IMU_calibrate(self->imu);
            break;
        default: break;
    }
}

struct FlightController *FlightController_create(struct IMU *imu, struct Rotor *rotors[], uint8_t num_rotors, struct CommandHandler *ch)
{
    struct FlightController *result = (struct FlightController *)malloc(sizeof(struct FlightController));

    if (result)
    {
        adc_init();
        //get_sensor_data();

        result->imu = imu;
        result->loop = LoopHandler_init_controlblock(FlightController_loop);
        result->mode = Init;
        result->debug_mode = false;
        result->on_changed_mode = NULL;
        result->on_changed_mode_internal = __FlightController_on_changed_mode;
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
        result->ch = ch;
        result->phi_offset = phi;
        result->theta_offset=theta;
        result->throttle = 0;
        result->yaw_rate = 0;
        result->pitch_angle = 0;
        result->roll_angle = 0;
        result->hold_throttle = 0;
        //result->hold_throttle_raw = 0;
        result->P = 18;
        result->P1 = 30;
        result->P2 = 80;
        result->H = 10;
    }

    return result;
}

bool FlightController_change_mode(struct FlightController *self, enum FlightControllerMode mode)
{
    if (self)
    {

        if (self->mode != mode && (FlightController_check_rotors_safe(self) || mode == Panic  || (self->mode == HoldHeight && mode == Full) || (self->mode == Full && mode == HoldHeight)))
        {
            // We can only change from panic over to safe
            if (self->mode != Panic || (self->mode == Panic && mode == Safe))
            {
                self->on_changed_mode_internal(self, mode, self->mode);

                if (self->on_changed_mode)
                {
                    self->on_changed_mode(self, mode, self->mode);
                }

                // might not be necessary
                self->input_ts = 0;

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
        self->pitch_angle = pitch_rate - 127;
        self->roll_angle = roll_rate - 127;
        self->throttle = throttle;

        self->input_ts = get_time_us();
    }
}

void FlightController_set_params(struct FlightController *self, uint8_t pid , uint8_t pvalue)
{
    if (self)
    {
        switch(pid){
            case 0:
                self->P = pvalue;
                DEBUG(0, "P:%d", pvalue);
                break;
            case 1:
                self->P1 = pvalue;
                DEBUG(0, "P1:%d", pvalue);
                break;
            case 2:
                self->P2 = pvalue;
                DEBUG(0, "P2:%d", pvalue);
                break;
            case 3:
                self->H = pvalue;
                DEBUG(0,"H:%d",pvalue);
                break;
            default:
                break;
        }

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

uint16_t FlightController_map_proportional(struct FlightController *self)
{
    uint16_t t;
    if (self->throttle >0)
    {
        t= ((self->throttle - 0) * (MAX_THROTTLE - MINIMUM_RPM) /255)+ MINIMUM_RPM;
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

uint16_t FlightController_sqrt_index_bounds(int16_t rpm_in)
{
    if (rpm_in> 2499){
        rpm_in = 2499;
    }
    if (rpm_in < 0){
        rpm_in = 0 ;
    }
    return rpm_in;
}
