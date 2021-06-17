//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_IMU_H
#define QUADCOPTER_FCB_IMU_H

#define BARO_WIN 5
#define BAT_WIN 5
#define BUTTERWORTH_N 3

#define P2PHI float2fix(1)
#define C1 float2fix(1)
#define C2 float2fix(1000)

#include "LoopHandler.h"

#include <stdbool.h>
#include <stdint.h>
#include "../hal/barometer.h"
#include "../hal/adc.h"

enum IMU_state
{
    IMU_Init,
    IMU_Measuring,
    IMU_MeasuringRaw,
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

    int16_t p;
    int16_t q;
    int16_t r;

    //acc readings filtered or raw
    int16_t measured_ax;
    int16_t measured_ay;
    int16_t measured_az;


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
    int32_t barometer_to_hold;
    int16_t imu_height_rate;

    //battery
    uint16_t battery_voltage[BAT_WIN];
    uint8_t battery_iterator;
    uint16_t battery_average;

    //butterworth
    int32_t sp_x[BUTTERWORTH_N];
    int16_t sq_x[BUTTERWORTH_N];
    int16_t sr_x[BUTTERWORTH_N];

    int32_t sp_y[BUTTERWORTH_N];
    int16_t sq_y[BUTTERWORTH_N];
    int16_t sr_y[BUTTERWORTH_N];

    int16_t a0;
    int16_t a1;
    int16_t a2;

    int16_t b0;
    int16_t b1;
    int16_t b2;

    //kalman
    int32_t bias_phi;
    int32_t p_estimate;
    int32_t e_phi;
    int32_t phi_kalman;

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
void IMU_go_raw(struct IMU *self);
void IMU_go_full(struct IMU *self);

int32_t     float2fix(double x);
int32_t 	fix2float(int x);
double 	fixmul(int x1, int x2);

#endif //QUADCOPTER_FCB_IMU_H
