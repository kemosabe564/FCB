//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_FLIGHTCONTROLLER_H
#define QUADCOPTER_FCB_FLIGHTCONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "LoopHandler.h"
#include "Rotor.h"
#include "IMU.h"
#include "CommandHandler.h"
#include "Command.h"

#define MINIMUM_RPM 180

#define SQRT_SCALE_BACK 17
#define BAT_THRESHOLD 1050


static const int get_sqrt[] = {0,1,1,1,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31};

enum FlightControllerMode
{
    Init = 1,
    Safe = 2,
    Panic = 3,
    Manual = 4,
    Calibrate = 5,
    Yaw = 6,
    Full = 7,
    Raw = 8,
    HoldHeight = 9
};

struct FlightController;

typedef void (*FlightControllerChangedMode)(struct FlightController *self, enum FlightControllerMode new_mode, enum FlightControllerMode old_mode);

struct FlightController
{
    struct Rotor **rotors;
    uint8_t num_rotors;
    enum FlightControllerMode mode;
    struct IMU *imu;
    bool debug_mode;

    int16_t yaw_rate;
    int16_t pitch_angle;
    int16_t roll_angle;
    uint16_t throttle;
    uint32_t input_ts;

    int16_t current_psi;
    int16_t previous_psi;

    int16_t current_phi;
    int16_t previous_phi;

    int16_t current_theta;
    int16_t previous_theta;

    uint32_t calibrate_start_time;
    int16_t phi_offset;
    int16_t theta_offset;
    bool is_calibrating;

    uint8_t P ;
    uint8_t P1 ;
    uint8_t P2 ;

    struct CommandHandler *ch;

    FlightControllerChangedMode on_changed_mode;
    FlightControllerChangedMode on_changed_mode_internal;

    struct LoopHandlerControlBlock loop;
};

struct FlightController *FlightController_create(struct IMU *imu, struct Rotor *rotors[], uint8_t num_rotors,struct CommandHandler *ch);

char *FlightControllerMode_to_str(enum FlightControllerMode mode);

bool FlightController_change_mode(struct FlightController *self, enum FlightControllerMode mode);
void FlightController_set_on_change_mode(struct FlightController *self, FlightControllerChangedMode handler);
void __FlightController_on_changed_mode(struct FlightController *self, enum FlightControllerMode new_mode, enum FlightControllerMode old_mode);
bool FlightController_check_rotors_safe(struct FlightController *self);
void FlightController_set_throttle(struct FlightController *self, uint16_t throttle);
void FlightController_set_controls(struct FlightController *self, int16_t yaw_rate, int16_t pitch_rate, int16_t roll_rate, uint16_t throttle);
void FlightController_set_params(struct FlightController *self, uint8_t pid , uint8_t pvalue);
void FlightController_loop(void *context, uint32_t delta_us);
uint16_t FlightController_map_proportional(struct FlightController *self);
void FlightController_destroy(struct FlightController *self);
uint16_t FlightController_set_limited_rpm(uint16_t rpm);
uint16_t FlightController_map_throttle(struct  FlightController *self);
int16_t FlightController_roll_over_angle(int16_t angle);
uint16_t FlightController_sqrt_index_bounds(uint16_t rpm_in);

#endif //QUADCOPTER_FCB_FLIGHTCONTROLLER_H
