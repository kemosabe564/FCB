//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_IMU_H
#define QUADCOPTER_FCB_IMU_H

#define BARO_WIN 5
#define BAT_WIN 5
#define BUTTERWORTH_N 3
#define fixedpoint 18
#define K (1 << (fixedpoint - 1))


// P2PHI = 0.0081, fp is 132
#define P2PHI 2123
// #define C1_P float2fix(1024)
// #define C2_P float2fix(1250*C1)
#define C1_P_inv 256
#define C2_P_P2PHI_inv 25

// P2PHI = 0.0081, fp is 132
#define Q2THETA 132
// #define C1_Q float2fix(256)
// #define C2_Q float2fix(1000000)
#define C1_Q_inv float2fix(256)
#define C2_Q_Q2THETA_inv float2fix(1000000)

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
    int64_t sp_x[BUTTERWORTH_N];
    int64_t sq_x[BUTTERWORTH_N];
    int64_t sr_x[BUTTERWORTH_N];

    int64_t sp_y[BUTTERWORTH_N];
    int64_t sq_y[BUTTERWORTH_N];
    int64_t sr_y[BUTTERWORTH_N];

    int64_t sax_x[BUTTERWORTH_N];
    int64_t say_x[BUTTERWORTH_N];

    int64_t sax_y[BUTTERWORTH_N];
    int64_t say_y[BUTTERWORTH_N];

    //filter
    int64_t a0;
    int64_t a1;
    int64_t a2;

    int64_t b0;
    int64_t b1;
    int64_t b2;

    //kalman
    int64_t bias_phi;
    int64_t p_estimate;
    int64_t e_phi;
    int64_t phi_kalman;

    int64_t bias_theta;
    int64_t q_estimate;
    int64_t e_theta;
    int64_t theta_kalman;

    bool calibrated;
    uint32_t calibration_start_ts;
    uint32_t calibration_time_us;
    bool dmp_enabled;
    uint16_t frequency;

    uint32_t base_time;
};

void IMU_loop(void *context, uint32_t delta_us);

struct IMU *IMU_create(bool dmp, uint16_t frequency);
void IMU_calibrate(struct IMU *self);
void IMU_destroy(struct IMU *self);
void IMU_go_raw(struct IMU *self);
void IMU_go_full(struct IMU *self);

int64_t     float2fix(int x);
int 	    fix2float(int64_t x);
int64_t 	fixmul(int64_t x1, int64_t x2);

#endif //QUADCOPTER_FCB_IMU_H
