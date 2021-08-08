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

    if (result) //authored by Vivian
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
            //
            result->sp_x[i]=0;
            result->sp_y[i]=0;
            //
            result->sq_x[i]=0;
            result->sq_y[i]=0;
            //
            result->sr_x[i]=0;
            result->sr_y[i]=0;
            //
            result->sax_x[i]=0;
            result->sax_y[i]=0;
            //
            result->say_x[i]=0;
            result->say_y[i]=0;

        }

        //need to replace these with MATLAB constants

        // result->a0 = float2fix(0.0095257623);
        // result->a1 = float2fix(0.0190515247);
        // result->a2 = float2fix(0.0095257623);
        // result->b0 = float2fix(1);
        // result->b1 = float2fix(-1.705552145);
        // result->b2 = float2fix(0.743655195);

        //b1 b2 -1.91058270331900	0.914412856345180
        //a0 a1 a2 0.000957538256545570	0.00191507651309114	0.000957538256545570

        //for yaw 
        result->a0 = 2497;//60;
        result->a1 = 4994;//77;
        result->a2 = 2497;//60;
        result->b0 = 1;
        result->b1 = -447100;//-29812;
        result->b2 = 194944;//13672;

        //for roll and pitch
        result->A0 = 251;//60;
        result->A1 = 502;//77;
        result->A2 = 251;//60;
        result->B0 = 1;
        result->B1 = -500847;//-29812;
        result->B2 = 239707;//13672;

        //kalman

        result->bias_phi = 0;
        result->e_phi = 0;
        result->phi_kalman = 0;
        result->p_estimate = 0;

        result->bias_theta = 0;
        result->e_theta = 0;
        result->theta_kalman = 0;
        result->q_estimate = 0;

    }

    return result;
}

// authored by Nathan
void IMU_loop(void *context, uint32_t delta_us)
{
    struct IMU *imu = (struct IMU *)context;

    switch (imu->state)
    {
        case IMU_Init: {
//            imu_init(imu->dmp_enabled, imu->frequency);
//            timers_init();

            imu->state = IMU_Measuring;
            imu->state = IMU_MeasuringRaw;

            imu->base_time = get_time_us();

        }
            break;
        case IMU_Measuring: { //authored by Vivian

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

        case IMU_MeasuringRaw: //authored by Vivian and Ting
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

            // TODO:
            // 1. fix fp to int64_t
            // 2. verify it with buttorworth filter
            // 3. apply it to Kalman filter equation
            // 4. adjust parameters

            // above things are all done

            // TODO: 8.6
            // 1. filter of sax and say
            // 2. adjust parameters


            //**** MEASURING RAW ****

            //sp
            // //moving all values and reading the new input
            // imu->sp_x[2]=imu->sp_x[1];
            // imu->sp_x[1]=imu->sp_x[0];

            imu->sp_x[0] = float2fix(sp);
            
            // //moving all y values
            // imu->sp_y[2]=imu->sp_y[1];
            // imu->sp_y[1]=imu->sp_y[0];
            // imu->sp_y[0] = fixmul(imu->a0,imu->sp_x[0])+fixmul(imu->a1,imu->sp_x[1])+fixmul(imu->a2,imu->sp_x[2])-
            //                fixmul(imu->b1,imu->sp_y[1])-fixmul(imu->b2,imu->sp_y[2]);
            // //DEBUG(0,"sp%d",sp);

            //sq
            // imu->sq_x[2]=imu->sq_x[1];
            // imu->sq_x[1]=imu->sq_x[0];
            imu->sq_x[0] = float2fix(sq);
            // //DEBUG(0,"sq%d",sq);
            // //moving all y values
            // imu->sq_y[2]=imu->sq_y[1];
            // imu->sq_y[1]=imu->sq_y[0];
            // imu->sq_y[0] = fixmul(imu->a0,imu->sq_x[0])+fixmul(imu->a1,imu->sq_x[1])+fixmul(imu->a2,imu->sq_x[2])-
            //                fixmul(imu->b1,imu->sq_y[1])-fixmul(imu->b2,imu->sq_y[2]);

            //sr
            imu->sr_x[2]=imu->sr_x[1];
            imu->sr_x[1]=imu->sr_x[0];
            imu->sr_x[0] = float2fix(sr);
            //moving all y values
            imu->sr_y[2]=imu->sr_y[1];
            imu->sr_y[1]=imu->sr_y[0];
            imu->sr_y[0] = fixmul(imu->a0,imu->sr_x[0])+fixmul(imu->a1,imu->sr_x[1])+fixmul(imu->a2,imu->sr_x[2])-
                           fixmul(imu->b1,imu->sr_y[1])-fixmul(imu->b2,imu->sr_y[2]);

            //sax
            imu->sax_x[2]=imu->sax_x[1];
            imu->sax_x[1]=imu->sax_x[0];
            imu->sax_x[0] = float2fix(sax);
            //moving all y values
            imu->sax_y[2]=imu->sax_y[1];
            imu->sax_y[1]=imu->sax_y[0];
            imu->sax_y[0] = fixmul(imu->a0,imu->sax_x[0])+fixmul(imu->a1,imu->sax_x[1])+fixmul(imu->a2,imu->sax_x[2])-
                            fixmul(imu->b1,imu->sax_y[1])-fixmul(imu->b2,imu->sax_y[2]);

            //say
            imu->say_x[2]=imu->say_x[1];
            imu->say_x[1]=imu->say_x[0];
            imu->say_x[0] = float2fix(say);
            //moving all y values
            imu->say_y[2]=imu->say_y[1];
            imu->say_y[1]=imu->say_y[0];
            imu->say_y[0] = fixmul(imu->a0,imu->say_x[0])+fixmul(imu->a1,imu->say_x[1])+fixmul(imu->a2,imu->say_x[2])-
                            fixmul(imu->b1,imu->say_y[1])-fixmul(imu->b2,imu->say_y[2]);


            // imu->p = fix2float(imu->sp_y[0]) - imu->sp_offset;
            // imu->q = fix2float(imu->sq_y[0]) - imu->sq_offset;
            // imu->r = fix2float(imu->sr_y[0]) - imu->sr_offset;

            // imu->p = fix2float(imu->sp_y[0]);
            // imu->q = fix2float(imu->sq_y[0]);
            // imu->r = fix2float(imu->sr_y[0]);            
            imu->p = sp;
            imu->q = sq;
            imu->r = sr;    

            //kalman for phi and theta

            //p = sp - b
            imu->p_estimate = imu->sp_x[0] - imu->bias_phi;
            imu->q_estimate = imu->sq_x[0] - imu->bias_theta;
            // imu->p_estimate = float2fix(sp) - imu->bias_phi;
            // imu->q_estimate = float2fix(sq) - imu->bias_theta;

            // DEBUG(0, "p: %d",fix2float(imu->sp_x[0]));
            // DEBUG(0, "sax: %d",fix2float(imu->sax_y[0]));


            //phi = phi + p * P2PHI
            imu->phi_kalman = imu->phi_kalman + fixmul(imu->p_estimate,P2PHI);
            imu->theta_kalman = imu->theta_kalman + fixmul(imu->q_estimate,Q2THETA);
            //phi_p(i) = phi_p(i-1) + p(i-1) * p2phi; 


            //e = phi – sphi
            imu->e_phi = imu->phi_kalman - (imu->say_y[0]);
            imu->e_theta = imu->theta_kalman - (imu->sax_y[0]);  

            // imu->e_phi = imu->phi_kalman - float2fix(say);
            // imu->e_theta = imu->theta_kalman - float2fix(sax);            
            // imu->e_phi = imu->phi_kalman - float2fix(phi);
            // imu->e_theta = imu->theta_kalman - float2fix(theta);

            //phi = phi – e / C1
            imu->phi_kalman = imu->phi_kalman - fixmul(imu->e_phi, C1_P_inv);
            imu->theta_kalman = imu->theta_kalman - fixmul(imu->e_theta, C1_Q_inv);

            //b = b + (e/P2PHI) / C2
            // DEBUG(0, "before: %d",imu->bias_phi);
            imu->bias_phi = imu->bias_phi + fixmul(imu->e_phi, C2_P_P2PHI_inv);
            // DEBUG(0, "after: %d",imu->bias_phi);
            imu->bias_theta = imu->bias_theta + fixmul(imu->e_theta,C2_Q_Q2THETA_inv);



            //****END MEASURING RAW ****

            // for all printing

            // check output of filter
            // DEBUG(0, "x:%d", fix2float(imu->say_x[0]));
            // DEBUG(0, "y:%d", fix2float(imu->say_y[0]));

            // check output of Kalman
            // uint32_t time_now = get_time_us();
            // DEBUG(0, "t:%d",imu->base_time);
            DEBUG(0, "phi:%d",fix2float(imu->phi_kalman));
            DEBUG(0, "p:%d", fix2float(imu->sp_x[0]));
            DEBUG(0, "say:%d", fix2float(imu->say_y[0]));
            
            // other checking
            // DEBUG(0, "q: %d",imu->q);
            // DEBUG(0, "q: %d",imu->q);
            // DEBUG(0, "r: %d",imu->r);


            //setting the values
            imu->roll_angle = fix2float(imu->phi_kalman);
            imu->pitch_angle = fix2float(imu->theta_kalman);
            
            imu->roll_angle = imu->say_y[0];
            imu->pitch_angle = imu->sax_y[0];
            imu->yaw_rate = imu->r;
        }
            break;

        case IMU_StartCalibration: { //authored by Nathan
            imu->calibration_start_ts = get_time_us();
            imu->state = IMU_Waiting;
        }
            break;
        case IMU_Waiting: { //authored by Nathan
            uint32_t now = get_time_us();

            if ((now - imu->calibration_start_ts) >= imu->calibration_time_us)
            {
                imu->state = IMU_FinishCalibration;
            }
        }
            break;
        case IMU_FinishCalibration: { //authored by Vivian
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
//authored by Nathan
void IMU_calibrate(struct IMU *self)
{
    if (self)
    {
        self->calibrated = false;
        self->state = IMU_StartCalibration;
    }
}
//authored by Nathan
void IMU_destroy(struct IMU *self)
{
    if (self)
    {
        free(self);
    }
}
//authored by Vivian
void IMU_go_raw(struct IMU *self)
{
    imu_init(false, 300);
    self->state = IMU_MeasuringRaw;
}
//authored by Vivian
void IMU_go_full(struct IMU *self)
{
    imu_init(true, 100);
    self->state = IMU_Measuring;
}



/*----------------------------------------------------------------
 * float2fix -- convert float to fixed point 18+14 bits
 *----------------------------------------------------------------
 */
//authored by Ting
//Note : These are taken directly from the example on the course website
int64_t float2fix(int x)
{
    int64_t result;
    result = x * (1 << fixedpoint);
    return result;
}


/*----------------------------------------------------------------
 * fix2float -- convert fixed 18+14 bits to float
 *----------------------------------------------------------------
 */
//authored by Ting
//Note : These are taken directly from the example on the course website
int fix2float(int64_t x)
{
    // double	y;

    // y = ((double) x) / (1 << 14);
    // return (int32_t) y;
    int result;
    result = x >> fixedpoint;
    // if(((a >> (fixedpoint-1)) & 1 )== 1)
    // {
    //     result ++;
    // }
    return result;    
}


/*----------------------------------------------------------------
 * fixmul -- multiply fixed 18+14 bits to float
 *----------------------------------------------------------------
 */
//authored by Ting
//Note : These are taken directly from the example on the course website
int64_t fixmul(int64_t a, int64_t b)
{
    // int	y;
    // y = x1 * x2; // Note: be sure this fits in 32 bits !!!!
    // y = (y >> 14);
    // return y;

    int64_t result;
    int64_t temp;
    temp = a * b; 
    temp += K;
    result = (int64_t)(temp >> fixedpoint);
    return result;    
}
