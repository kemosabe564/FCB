//
// Created by nathan on 26-04-21.
//

#include "IMU.h"

#include <stdlib.h>
#include "../mpu6050/mpu6050.h"
#include "../hal/timers.h"
#include "Debug.h"
#include <stdio.h>

struct IMU *IMU_create(bool dmp, uint16_t frequency)
{
    struct IMU *result = (struct IMU *)malloc(sizeof(struct IMU));

    if (result)
    {
        result->loop = LoopHandler_init_controlblock(IMU_loop);
        result->state = IMU_Init;

        result->roll_angle = 0;
        result->pitch_angle = 0;
        result->yaw_rate = 0;

        result->imu_psi_rate = 0;
        result->imu_theta_rate = 0;
        result->imu_phi_rate =0;

        result->measured_p = 0;
        result->measured_q = 0;
        result->measured_r = 0;

        result->measured_ax = 0;
        result->measured_ay = 0;
        result->measured_az = 0;

        result->sp_offset = 0;
        result->sq_offset = 0;
        result->sr_offset = 0;

        result->roll_angle_offset = 0;
        result->pitch_angle_offset = 0;

        for(int i=0; i<BARO_WIN; i++)
        {
            result->barometer_readings[i]=102400;
        }
        for (int i=0; i<BAT_WIN; i++)
        {
            result->battery_voltage[i]=1100;
        }
        result->battery_iterator = 0;
        result->battery_average = 1100;

        result->barometer_average = 102400;
        result->barometer_to_hold = 102000;
        result->barometer_iterator = 0;
        result->imu_height_rate = 0;

        result->calibrated = false;
        result->calibration_start_ts = 0;
        result->calibration_time_us = 10000000;
        result->dmp_enabled = dmp;
        result->frequency = frequency;

        //initializing all readings to 0
        for(int i=0; i<BUTTERWORTH_N; i++)
        {
            result->sp_x[i]=0;
            result->sq_x[i]=0;
            result->sr_x[i]=0;
            result->sp_y[i]=0;
            result->sq_y[i]=0;
            result->sr_y[i]=0;
        }

        //need to replace these with MATLAB constants
        //on debugging this a0 became 329 in int
        //that is correct

        result->a0 = float2fix(0.020083372);
        result->a1 = float2fix(0.040166743);
        result->a2 = float2fix(0.020083372);
        result->b0 = float2fix(1);
        result->b1 = float2fix(-1.5610181);
        result->b2 = float2fix(0.6413515);




    }

    return result;
}

void IMU_loop(void *context, uint32_t delta_us)
{
    struct IMU *imu = (struct IMU *)context;

    switch (imu->state)
    {
        case IMU_Init: {
//            imu_init(imu->dmp_enabled, imu->frequency);
//            timers_init();

            imu->state = IMU_Measuring;
        }
            break;
        case IMU_Measuring: {
            if (check_sensor_int_flag())
            {
                get_sensor_data();
            }
            adc_request_sample();
            read_baro();
            //decide if iterator index needs to loop back
            if (imu->battery_iterator >= BAT_WIN)
            {
                imu->battery_iterator=0;
            }
            //set the required index
            imu->battery_voltage[imu->battery_iterator]=bat_volt;
            imu->battery_iterator++;
            int32_t bat_sum =0;
            for (int i=0; i<BAT_WIN; i++)
            {
                bat_sum = bat_sum + imu->battery_voltage[i];
            }
            imu->battery_average = bat_sum/BAT_WIN;
            //This works , verified the average
            //DEBUG(0,"a%dr%d",imu->battery_average,bat_volt);


            imu->barometer_iterator++;
            if (imu->barometer_iterator >= BARO_WIN)
            {
                imu->barometer_iterator=0;
            }
            imu->barometer_readings[imu->barometer_iterator]=pressure;

            int32_t sum =0;
            for (int i=0; i<BARO_WIN; i++)
            {
                sum = sum + imu->barometer_readings[i];
            }


            imu->imu_height_rate = (sum/BARO_WIN) - imu->barometer_average;
            imu->barometer_average = sum/BARO_WIN;

            imu->imu_psi_rate = psi - imu->yaw_rate;
            imu->imu_phi_rate = phi - imu->roll_angle;
            imu->imu_theta_rate= theta - imu->pitch_angle;

            imu->roll_angle = phi;
            imu->pitch_angle = theta;
            imu->yaw_rate = psi;

            imu->measured_p = sp - imu->sp_offset;
            imu->measured_q = sq - imu->sq_offset;
            imu->measured_r = sr - imu->sr_offset;

            imu->measured_ax = sax;
            imu->measured_ay = say;
            imu->measured_az = saz;

            // adjusted angles. More processing might be added here
            //This can overflow - needs to be changed
            //roll over handled in fc
            imu->cal_roll_angle = imu->roll_angle - imu->roll_angle_offset;
            imu->cal_pitch_angle = imu->pitch_angle - imu->pitch_angle_offset;

            //BUTTERWORTH
            //DEBUG(0,"a%d",imu->a0); //works fine

            //moving all values and reading the new input
            imu->sp_x[2]=imu->sp_x[1];
            imu->sp_x[1]=imu->sp_x[0];
            imu->sp_x[0] = sp;

            //moving all y values
            imu->sp_y[2]=imu->sp_y[1];
            imu->sp_y[1]=imu->sp_y[0];

            //warning:be careful of terrible naming convention
            int16_t sp_x0 = float2fix(imu->sp_x[0]);
            int16_t sp_x1 = float2fix(imu->sp_x[1]);
            int16_t sp_x2 = float2fix(imu->sp_x[2]);
            int16_t sp_y1 = float2fix(imu->sp_y[1]);
            int16_t sp_y2 = float2fix(imu->sp_y[2]);

            int16_t sp_y0 = fixmul(imu->a0,sp_x0)+fixmul(imu->a1,sp_x1)+fixmul(imu->a2,sp_x2)-
                     fixmul(imu->b1,sp_y1)-fixmul(imu->b2,sp_y2);
            imu->sp_y[0]= fix2float(sp_y0);
            DEBUG(0,"y%d",imu->sp_y[0]);
        }
            break;
        case IMU_StartCalibration: {
            imu->calibration_start_ts = get_time_us();
            imu->state = IMU_Waiting;
        }
            break;
        case IMU_Waiting: {
            uint32_t now = get_time_us();

            if ((now - imu->calibration_start_ts) >= imu->calibration_time_us)
            {
                imu->state = IMU_FinishCalibration;
            }
        }
            break;
        case IMU_FinishCalibration: {
            if (check_sensor_int_flag())
            {
                get_sensor_data();

                imu->roll_angle_offset = phi;
                imu->pitch_angle_offset = theta;

                imu->sp_offset = sp;
                imu->sq_offset = sq;
                imu->sr_offset = sr;

                imu->calibrated = true; 
                imu->state = IMU_Measuring;
            }
        }
            break;
    }

}

void IMU_calibrate(struct IMU *self)
{
    if (self)
    {
        self->calibrated = false;
        self->state = IMU_StartCalibration;
    }
}

void IMU_destroy(struct IMU *self)
{
    if (self)
    {
        free(self);
    }
}

/*----------------------------------------------------------------
 * float2fix -- convert float to fixed point 18+14 bits
 *----------------------------------------------------------------
 */
int     float2fix(double x)
{
    int	y;

    y = x * (1 << 14);
    return y;
}


/*----------------------------------------------------------------
 * fix2float -- convert fixed 18+14 bits to float
 *----------------------------------------------------------------
 */
double 	fix2float(int x)
{
    double	y;

    y = ((double) x) / (1 << 14);
    return y;
}


/*----------------------------------------------------------------
 * fixmul -- multiply fixed 18+14 bits to float
 *----------------------------------------------------------------
 */
double 	fixmul(int x1, int x2)
{
    int	y;

    y = x1 * x2; // Note: be sure this fits in 32 bits !!!!
    y = (y >> 14);
    return y;
}