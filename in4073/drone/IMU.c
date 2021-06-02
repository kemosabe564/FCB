//
// Created by nathan on 26-04-21.
//

#include "IMU.h"

#include <stdlib.h>
#include "../mpu60580/mpu6050.h"
#include "../hal/timers.h"
#include "Debug.h"

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

        result->measured_p = 0;
        result->measured_q = 0;
        result->measured_r = 0;

        result->measured_ax = 0;
        result->measured_ay = 0;
        result->measured_az = 0;

        result->roll_angle_offset = 0;
        result->pitch_angle_offset = 0;


        result->calibrated = false;
        result->calibration_start_ts = 0;
        result->calibration_time_us = 10000000;
        result->dmp_enabled = dmp;
        result->frequency = frequency;

        imu_init(true, frequency);
        timers_init();
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

            imu->roll_angle = phi;
            imu->pitch_angle = theta;
            imu->yaw_rate = psi;

            imu->measured_p = sp;
            imu->measured_q = sq;
            imu->measured_r = sr;

            imu->measured_ax = sax;
            imu->measured_ay = say;
            imu->measured_az = saz;

            // adjusted angles. More processing might be added here
            //This can overflow - needs to be changed
            //roll over handled in fc
            imu->cal_roll_angle = imu->roll_angle - imu->roll_angle_offset;
            imu->cal_pitch_angle = imu->pitch_angle - imu->pitch_angle_offset;
            //imu->yaw_angle = imu->raw_yaw_angle - imu->yaw_angle_offset;

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
            get_sensor_data();

            imu->roll_angle_offset = phi;
            imu->pitch_angle_offset = theta;

            imu->calibrated = true;
            imu->state = IMU_Measuring;
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