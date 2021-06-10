//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_IMU_H
#define QUADCOPTER_FCB_IMU_H

#define BARO_WIN 5
#define BAT_WIN 5

#include "LoopHandler.h"

#include <stdbool.h>
#include <stdint.h>
#include "../hal/barometer.h"
#include "../hal/adc.h"

enum IMU_state
{
    IMU_Init,
    IMU_Measuring,
    IMU_StartCalibration,
    IMU_Waiting,
    IMU_FinishCalibration,
};

struct IMU
{
    struct LoopHandlerControlBlock loop;

    enum IMU_state state;

    //processed angles
    int16_t roll_angle;
    int16_t pitch_angle;
    int16_t yaw_rate;

    int16_t imu_psi_rate;
    int16_t imu_theta_rate;
    int16_t imu_phi_rate;

    //gyro readings filtered or raw
    int16_t measured_p;
    int16_t measured_q;
    int16_t measured_r;

    //acc readings filtered or raw
    int16_t measured_ax;
    int16_t measured_ay;
    int16_t measured_az;

    //calibrated angles
    int16_t cal_roll_angle;
    int16_t cal_pitch_angle;

    // calibration data
    int16_t roll_angle_offset;
    int16_t pitch_angle_offset;

    int16_t sp_offset;
    int16_t sq_offset;
    int16_t sr_offset;

    //height  holding
    int32_t barometer_readings[BARO_WIN];
    int32_t barometer_average;
    uint8_t barometer_iterator;
    int16_t imu_height_rate;

    //battery
    uint16_t battery_voltage[BAT_WIN];
    uint8_t battery_iterator;
    uint16_t battery_average;

    bool calibrated;
    uint32_t calibration_start_ts;
    uint32_t calibration_time_us;
    bool dmp_enabled;
    uint16_t frequency;
};

void IMU_loop(void *context, uint32_t delta_us);

struct IMU *IMU_create(bool dmp, uint16_t frequency);
void IMU_calibrate(struct IMU *self);
void IMU_destroy(struct IMU *self);

#endif //QUADCOPTER_FCB_IMU_H
