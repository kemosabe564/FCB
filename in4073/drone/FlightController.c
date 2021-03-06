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

    //battery check authored by Vivian
    if (self->battery_check && (self->imu->battery_average < BAT_THRESHOLD && self->mode != Safe && self->mode != Panic))
    {
        FlightController_change_mode(self,Panic);
        DEBUG(0, "BAT LOW: %d ", self->imu->battery_average);
    }
    //battery warning authored by Vivian
    if (self->battery_check && !self->battery_warned && (self->imu->battery_average < BAT_WARN && self->imu->battery_average > BAT_THRESHOLD && self->mode != Safe && self->mode != Panic))
    {
        DEBUG(0,"BAT WARN: %d",self->imu->battery_average);
        self->battery_warned = true;
    }

    switch (self->mode) {
        case Init: //authored by Nathan
            self->debug_mode = (bat_volt == 0);

            FlightController_change_mode(self, Safe);
            break;
        case Safe: //authored by Vivian
            for (int i =0; i <self->num_rotors; i++)
            {
                Rotor_set_rpm(self->rotors[i],0);
            }

            break;
        case Panic: { //authored by Vivian
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
        case Manual: { //authored by Vivian
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
                uint16_t rpm1 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - self->roll_angle + self->yaw_rate)]);
                uint16_t rpm2 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - self->pitch_angle - self->yaw_rate)]);
                uint16_t rpm3 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + self->roll_angle + self->yaw_rate)]);

                //CommandHandler_send_command(self->ch, Command_make_debug_msg("rpm0 %d\n",rpm0));

                Rotor_set_rpm(self->rotors[0], rpm0);
                Rotor_set_rpm(self->rotors[1], rpm1);
                Rotor_set_rpm(self->rotors[2], rpm2);
                Rotor_set_rpm(self->rotors[3], rpm3);
                //DEBUG(0, "MOTORS %d %d %d %d", rpm0 ,rpm1,rpm2,rpm3);

            }
    
        }
            break;
        case Calibrate: //authored by Vivian
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
        case Yaw: { //authored by Vivian
            int16_t t = FlightController_map_proportional(self);
            //get set point
            int16_t yaw_setPoint = self->yaw_rate * 10;
            //get sensor reading
            int16_t  psi_rate = self->imu->r;
            //calculate error
            int16_t yaw_error = yaw_setPoint - psi_rate;
            //calculate compensation and apply
            int16_t yaw_compensation = (self->P * yaw_error) / 100;

            if (t<1)
            {
                for (int i =0; i <self->num_rotors; i++)
                {
                    Rotor_set_rpm(self->rotors[i],0);
                }
            }

            else
            {
                uint16_t rpm0 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + self->pitch_angle + yaw_compensation)]);
                uint16_t rpm1 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + self->roll_angle - yaw_compensation)]);
                uint16_t rpm2 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - self->pitch_angle + yaw_compensation)]);
                uint16_t rpm3 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - self->roll_angle - yaw_compensation)]);

                Rotor_set_rpm(self->rotors[0], rpm0);
                Rotor_set_rpm(self->rotors[1], rpm1);
                Rotor_set_rpm(self->rotors[2], rpm2);
                Rotor_set_rpm(self->rotors[3], rpm3);

            }

        }
            break;
        case Full: //authored by Vivian
        {
            //get throttle
            int16_t t = FlightController_map_proportional(self);
            //saving this in case we go into hold height
            //self->hold_throttle_raw = self->throttle;
            self->hold_throttle = t;
            self->imu->barometer_to_hold = self->imu->barometer_average;
            //get set points
            int16_t phi_setPoint = (self->roll_angle)/8;
            int16_t theta_setPoint = -(self->pitch_angle)/8;
            int16_t yaw_setPoint = self->yaw_rate * 10;
            //calculate rate of change

//            int16_t  psi_rate = self->imu->imu_psi_rate;
//            int16_t  phi_rate = self->imu->imu_phi_rate/5;
//            int16_t  theta_rate = self->imu->imu_theta_rate/5;

            int16_t  psi_rate = self->imu->r;
            //typical values -2000 to +2000
            int16_t  phi_rate = self->imu->p;
            int16_t  theta_rate = self->imu->q;



            //DEBUG( 0 , "PR %d", phi_rate);

            //calculate error1
            int16_t yaw_error = yaw_setPoint - psi_rate;
            //roll error verified
            //typical values are -15 to +15 for usual range
            int16_t roll_error = phi_setPoint - FlightController_roll_over_angle((self->imu->roll_angle - self->imu->roll_angle_offset)/ 256);
            //DEBUG(0,"RE%d",roll_error);
            int16_t  pitch_error = theta_setPoint - FlightController_roll_over_angle((self->imu->pitch_angle - self->imu->pitch_angle_offset)/ 256);

            //calculate compensation 1
            int16_t yaw_compensation = (self->P * yaw_error) / 100;
            int16_t roll_rate_setPoint = (self->P1 * roll_error);// / 10;
            int16_t pitch_rate_setPoint = (self->P1 * pitch_error);// / 10;

            //roll and pitch rate error
            int16_t roll_rate_error = roll_rate_setPoint - phi_rate;
            //DEBUG(0,"RE%d",roll_rate_error);
            int16_t pitch_rate_error = pitch_rate_setPoint - theta_rate;

            //calculate compensation 2
            int16_t roll_rate_compensation = -(self->P2 * roll_rate_error) / 100;
//            DEBUG(0,"RC%d",roll_rate_compensation);
            int16_t pitch_rate_compensation = -(self->P2 * pitch_rate_error) / 100;
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
                uint16_t rpm0 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + pitch_rate_compensation + yaw_compensation)]);
                uint16_t rpm1 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t + roll_rate_compensation - yaw_compensation)]);
                uint16_t rpm2 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - pitch_rate_compensation + yaw_compensation)]);
                uint16_t rpm3 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(t - roll_rate_compensation - yaw_compensation)]);

                Rotor_set_rpm(self->rotors[0], rpm0);
                Rotor_set_rpm(self->rotors[1], rpm1);
                Rotor_set_rpm(self->rotors[2], rpm2);
                Rotor_set_rpm(self->rotors[3], rpm3);

                //DEBUG(0, "%d,%d,%d", t, self->P1, self->P2);
                //DEBUG(0, "%d,%d,%d", phi_setPoint, roll_rate_error, roll_rate_compensation);
            }
        }
            break;
        case Raw: //authored by Vivian
            {
                //get throttle
                int16_t t = FlightController_map_proportional(self);
                //saving this in case we go into hold height
                //self->hold_throttle_raw = self->throttle;
                self->hold_throttle = t;
                self->imu->barometer_to_hold = self->imu->barometer_average;
                //get set points
                int16_t phi_setPoint = (self->roll_angle)/8;
                int16_t theta_setPoint = -(self->pitch_angle)/8;
                int16_t yaw_setPoint = self->yaw_rate * 10;
                //calculate rate of change

                int16_t  psi_rate = self->imu->r;
                //typical values -2000 to +2000
                int16_t  phi_rate = self->imu->p;
                int16_t  theta_rate = self->imu->q;

                //calculate error1
                int16_t yaw_error = yaw_setPoint - psi_rate;
                //roll error verified
                //typical values are -15 to +15 for usual range
                int16_t roll_error = phi_setPoint - FlightController_roll_over_angle((self->imu->roll_angle - self->imu->roll_angle_offset)/ 256);
                //DEBUG(0,"RE%d",roll_error);
                int16_t  pitch_error = theta_setPoint - FlightController_roll_over_angle((self->imu->pitch_angle - self->imu->pitch_angle_offset)/ 256);

                //calculate compensation 1
                int16_t yaw_compensation = (self->P * yaw_error) / 100;
                int16_t roll_rate_setPoint = (self->P1 * roll_error);// / 10;
                int16_t pitch_rate_setPoint = (self->P1 * pitch_error);// / 10;

                //roll and pitch rate error
                int16_t roll_rate_error = roll_rate_setPoint - phi_rate;
                //DEBUG(0,"RE%d",roll_rate_error);
                int16_t pitch_rate_error = pitch_rate_setPoint - theta_rate;

                //calculate compensation 2
                int16_t roll_rate_compensation = -(self->P2 * roll_rate_error) / 100;
//            DEBUG(0,"RC%d",roll_rate_compensation);
                int16_t pitch_rate_compensation = -(self->P2 * pitch_rate_error) / 100;
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

                }
        }


            break;
        case HoldHeight: //authored by Vivian
        {

            //get set points
            int16_t phi_setPoint = (self->roll_angle)/8;
            int16_t theta_setPoint = -(self->pitch_angle)/8;
            int16_t yaw_setPoint = self->yaw_rate * 10;

            int16_t  psi_rate = self->imu->r;
            int16_t  phi_rate = self->imu->p;
            int16_t  theta_rate = self->imu->q;

            //DEBUG( 0 , "PR %d", phi_rate);

            //calculate error1
            int16_t yaw_error = yaw_setPoint - psi_rate;
            //roll error verified
            //typical values are -15 to +15 for usual range
            int16_t roll_error = phi_setPoint - FlightController_roll_over_angle((self->imu->roll_angle - self->imu->roll_angle_offset)/ 256);
            //DEBUG(0,"RE%d",roll_error);
            int16_t  pitch_error = theta_setPoint - FlightController_roll_over_angle((self->imu->pitch_angle - self->imu->pitch_angle_offset)/ 256);

            //calculate compensation 1
            int16_t yaw_compensation = (self->P * yaw_error) / 100;
            int16_t roll_rate_setPoint = (self->P1 * roll_error);// / 10;
            int16_t pitch_rate_setPoint = (self->P1 * pitch_error);// / 10;

            //roll and pitch rate error
            int16_t roll_rate_error = roll_rate_setPoint - phi_rate;
            //DEBUG(0,"RE%d",roll_rate_error);
            int16_t pitch_rate_error = pitch_rate_setPoint - theta_rate;

            //calculate compensation 2
            int16_t roll_rate_compensation = -(self->P2 * roll_rate_error) / 100;
//            DEBUG(0,"RC%d",roll_rate_compensation);
            int16_t pitch_rate_compensation = -(self->P2 * pitch_rate_error) / 100;


            //get height rate
            //calculated in self->imu->imu_height_rate

            //error is 0 - rate
            //int16_t height_error = 0 - self->imu->imu_height_rate;
            int32_t  height_error = self->imu->barometer_average - self->imu->barometer_to_hold;
            int16_t lift_compensation = (self->H  * height_error ) /10;

            //DEBUG(0,"lc%d",lift_compensation);

            //DEBUG(0,"%d %d %d",self->imu->barometer_average,self->imu->imu_height_rate,lift_compensation);


            //increase or decrease lift
            uint16_t rpm0 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(self->hold_throttle + lift_compensation + pitch_rate_compensation + yaw_compensation)]);
            uint16_t rpm1 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(self->hold_throttle + lift_compensation + roll_rate_compensation - yaw_compensation)]);
            uint16_t rpm2 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(self->hold_throttle + lift_compensation - pitch_rate_compensation+ yaw_compensation)]);
            uint16_t rpm3 = FlightController_set_limited_rpm(SQRT_SCALE_BACK * get_sqrt[FlightController_sqrt_index_bounds(self->hold_throttle + lift_compensation - roll_rate_compensation - yaw_compensation)]);

            Rotor_set_rpm(self->rotors[0], rpm0);
            Rotor_set_rpm(self->rotors[1], rpm1);
            Rotor_set_rpm(self->rotors[2], rpm2);
            Rotor_set_rpm(self->rotors[3], rpm3);

        }
            break;
            
    
        
        case EndFlight_Logging:
        {
            FlightController_change_mode(self,EndFlight_Logging);
        }
            break;

        default:
            break;
    }


}
//authored by Nathan
void __FlightController_on_changed_mode(struct FlightController *self, enum FlightControllerMode new_mode, enum FlightControllerMode old_mode)
{
    switch (new_mode) {
        case Safe:
        case Panic: {
            self->yaw_rate = 0;
            self->pitch_angle = 0;
            self->roll_angle = 0;
        }
            break;
        case Calibrate:
            IMU_calibrate(self->imu);
            break;
        case Full:
        {
//            IMU_go_full(self->imu);
        }
            break;
        case Raw:
        {
            IMU_go_raw(self->imu);
        }
            break;
        default: break;
    }
}
//authored by Nathan
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
        result->rotors = (struct Rotor **)malloc(num_rotors * sizeof(struct Rotor *));
        memcpy(result->rotors, rotors, num_rotors * sizeof(struct Rotor *));
        result->ch = ch;
        result->phi_offset = phi;
        result->theta_offset=theta;
        result->throttle = 0;
        result->yaw_rate = 0;
        result->pitch_angle = 0;
        result->roll_angle = 0;
        result->hold_throttle = 0;
        //result->hold_throttle_raw = 0;
        result->battery_check = true;
        result->battery_warned = false;
        result->P = 11;
        result->P1 = 50;
        result->P2 = 20;
        result->H = 5;
    }

    return result;
}
//authored by Nathan
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

                self->mode = mode;
                return true;
            }
        }
    }
    return false;
}
//authored by Nathan
void FlightController_set_on_change_mode(struct FlightController *self, FlightControllerChangedMode handler)
{
    if (self)
    {
        self->on_changed_mode = handler;
    }
}
//authored by Nathan
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

//authored by Nathan
void FlightController_set_throttle(struct FlightController *self, uint16_t throttle)
{
    if (self)
    {
        self->throttle = throttle;
    }
}

//authored by Vivian
void FlightController_set_controls(struct FlightController *self, int16_t yaw_rate, int16_t pitch_rate, int16_t roll_rate, uint16_t throttle)
{
    if (self)
    {
        self->yaw_rate = yaw_rate - 127;
        self->pitch_angle = pitch_rate - 127;
        self->roll_angle = roll_rate - 127;
        self->throttle = throttle;
    }
}
//authored by Vivian
void FlightController_set_params(struct FlightController *self, uint8_t pid , uint8_t pvalue)
{
    if (self)
    {
        switch(pid){
            case 0:
                self->P = pvalue;
                DEBUG(0, "P: %d", pvalue);
                break;
            case 1:
                self->P1 = pvalue;
                DEBUG(0, "P1: %d", pvalue);
                break;
            case 2:
                self->P2 = pvalue;
                DEBUG(0, "P2: %d", pvalue);
                break;
            case 3:
                self->H = pvalue;
                DEBUG(0,"H:%d",pvalue);
                break;
            case 5:
                self->battery_check = pvalue > 0;
                DEBUG(0,"battery check %s", self->battery_check ? "enabled" : "disabled");
                break;
            default:
                break;
        }

    }
}
//authored by Nathan
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
        case EndFlight_Logging: return "EndFlight_Logging";
    }
    return "";
}
//authored by Nathan
void FlightController_destroy(struct FlightController *self)
{
    if (self)
    {
        free(self);
    }
}
//authored by Vivian
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
//authored by Vivian
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
//authored by Vivian
uint16_t FlightController_set_limited_rpm(uint16_t rpm)
{
    if (rpm < MINIMUM_RPM)
    {
        rpm = MINIMUM_RPM;
    }
    return rpm;
}
//authored by Vivian
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
//authored by Vivian
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
