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

        result->p = 0;
        result->q = 0;
        result->r = 0;

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

        result->a0 = float2fix(0.0095257623);
        result->a1 = float2fix(0.0190515247);
        result->a2 = float2fix(0.0095257623);
        result->b0 = float2fix(1);
        result->b1 = float2fix(-1.705552145);
        result->b2 = float2fix(0.743655195);

        //kalman

        result->bias_phi = 0;
        result->e_phi = 0;
        result->phi_kalman = 0;
        result->p_estimate = 0;

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

            //****COMMON****

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

            //****END COMMON****
            //**** MEASURING FULL  *****

// sss asdasd ASAS

            imu->roll_angle = phi;
            imu->pitch_angle = theta;
            imu->yaw_rate = psi;

            imu->measured_p = sp;
            imu->measured_q = sq;
            imu->measured_r = sr;

            imu->p = imu->measured_p - imu->sp_offset;
            imu->q = imu->measured_q - imu->sq_offset;
            imu->r = imu->measured_r - imu->sr_offset;

            imu->measured_ax = sax;
            imu->measured_ay = say;
            imu->measured_az = saz;

            //**** END MEASURING FULL ****
        }
            break;

        case IMU_MeasuringRaw:
        {
            //****COMMON****

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

            //****END COMMON****
            //**** MEASURING RAW ****

            //sp
            //moving all values and reading the new input
            imu->sp_x[2]=imu->sp_x[1];
            imu->sp_x[1]=imu->sp_x[0];
            imu->sp_x[0] = float2fix(sq);

            //moving all y values
            imu->sp_y[2]=imu->sp_y[1];
            imu->sp_y[1]=imu->sp_y[0];
            imu->sp_y[0] = fixmul(imu->a0,imu->sp_x[0])+fixmul(imu->a1,imu->sp_x[1])+fixmul(imu->a2,imu->sp_x[2])-
                           fixmul(imu->b1,imu->sp_y[1])-fixmul(imu->b2,imu->sp_y[2]);
            //DEBUG(0,"sp%d",sp);

            //sq
            imu->sq_x[2]=imu->sq_x[1];
            imu->sq_x[1]=imu->sq_x[0];
            imu->sq_x[0] = float2fix(sp);
            //DEBUG(0,"sq%d",sq);

            //moving all y values
            imu->sq_y[2]=imu->sq_y[1];
            imu->sq_y[1]=imu->sq_y[0];

            imu->sq_y[0] = fixmul(imu->a0,imu->sq_x[0])+fixmul(imu->a1,imu->sq_x[1])+fixmul(imu->a2,imu->sq_x[2])-
                           fixmul(imu->b1,imu->sq_y[1])-fixmul(imu->b2,imu->sq_y[2]);

            //sr
            imu->sr_x[2]=imu->sr_x[1];
            imu->sr_x[1]=imu->sr_x[0];
            imu->sr_x[0] = float2fix(sr);

            //moving all y values
            imu->sr_y[2]=imu->sr_y[1];
            imu->sr_y[1]=imu->sr_y[0];

            imu->sr_y[0] = fixmul(imu->a0,imu->sr_x[0])+fixmul(imu->a1,imu->sr_x[1])+fixmul(imu->a2,imu->sr_x[2])-
                           fixmul(imu->b1,imu->sr_y[1])-fixmul(imu->b2,imu->sr_y[2]);

            imu->p = fix2float(imu->sp_y[0]) - imu->sp_offset;
            imu->q = fix2float(imu->sq_y[0]) - imu->sq_offset;
            imu->r = fix2float(imu->sr_y[0]) - imu->sr_offset;

            DEBUG(0, "p%d",imu->p);
            DEBUG(0, "q%d",imu->q);

            //kalman for phi and theta

            //p = sp - b
            imu->p_estimate = imu->sp_y[0] - imu->bias_phi;

            //phi = phi + p * P2PHI
            imu->phi_kalman = imu->phi_kalman + fixmul(imu->p_estimate,P2PHI);

            //e = phi – sphi
            imu->e_phi = imu->phi_kalman - phi;

            //phi = phi – e / C1
            imu->phi_kalman = imu->phi_kalman - (imu->e_phi / C1);

            //b = b + (e/P2PHI) / C2
            imu->bias_phi = imu->bias_phi + (imu->e_phi/P2PHI) / C2;

            //****END MEASURING RAW ****
            //DEBUG(0,"y%d", fix2float(imu->sp_y[0]));
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

void IMU_go_raw(struct IMU *self)
{
    imu_init(false, 300);
    self->state = IMU_MeasuringRaw;
}

void IMU_go_full(struct IMU *self)
{
    imu_init(true, 100);
    self->state = IMU_Measuring;
}


/*----------------------------------------------------------------
 * float2fix -- convert float to fixed point 18+14 bits
 *----------------------------------------------------------------
 */
int32_t    float2fix(double x)
{
    int32_t	y;

    y = x * (1 << 14);
    return y;
}


/*----------------------------------------------------------------
 * fix2float -- convert fixed 18+14 bits to float
 *----------------------------------------------------------------
 */
int32_t 	fix2float(int x)
{
    double	y;

    y = ((double) x) / (1 << 14);
    return (int32_t) y;
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