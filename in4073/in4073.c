/*------------------------------------------------------------------
 *  in4073.c -- test QR engines and sensors
 *
 *  reads ae[0-3] uart rx queue
 *  (q,w,e,r increment, a,s,d,f decrement)
 *
 *  prints timestamp, ae[0-3], sensors to uart tx queue
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  June 2016
 *------------------------------------------------------------------
 */
#include "in4073.h"
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "adc.h"
#include "barometer.h"
#include "gpio.h"
#include "spi_flash.h"
#include "timers.h"
#include "twi.h"
#include "hal/uart.h"
#include "control.h"
#include "mpu6050/mpu6050.h"
#include "utils/quad_ble.h"

#include "drone/LoopHandler.h"
#include "drone/Rotor.h"
#include "drone/IMU.h"
#include "drone/FlightController.h"

bool demo_done;

#define COMM_SERIAL 0
#define COMM_BLE    1

/*------------------------------------------------------------------
 * main -- everything you need is here :)
 *------------------------------------------------------------------
 */
int main(void)
{
    bool running = true;

    struct LoopHandler lh = LoopHandler_init();

    struct RotorMap r_map = RotorMap_init(0, 10000);

    struct Rotor r1 = Rotor_init(&r_map, MOTOR_0_PIN, -15,  15);
    struct Rotor r2 = Rotor_init(&r_map, MOTOR_1_PIN,  15,  15);
    struct Rotor r3 = Rotor_init(&r_map, MOTOR_2_PIN, -15, -15);
    struct Rotor r4 = Rotor_init(&r_map, MOTOR_3_PIN, -15,  15);

    struct IMU imu = IMU_init();

    struct FlightController fc = FlightController_init(&imu, { &r1, &r2, &r3, &r4 }, 4);

//    struct Comms ble_comms BLE_init();
//    struct Comms serial_comms Serial_init(115200);
//
//    struct Comm_Handler comm_handler = CommHandler_init(COMM_SERIAL, &fc);
//
//    CommHandler_add_comms(&comm_handler, &serial_comms, COMM_SERIAL);
//    CommHandler_add_comms(&comm_handler, &ble_comms, COMM_BLE);

	while (running)
	{
        LoopHandler_loop(&lh, LH_LINK(fc), 0);

        LoopHandler_loop(&lh, LH_LINK(r1), 100);
        LoopHandler_loop(&lh, LH_LINK(r2), 100);
        LoopHandler_loop(&lh, LH_LINK(r3), 100);
        LoopHandler_loop(&lh, LH_LINK(r4), 100);

        LoopHandler_loop(&lh, LH_LINK(serial_comms), 100);
        LoopHandler_loop(&lh, LH_LINK(ble_comms), 50);

        LoopHandler_loop(&lh, LH_LINK(comm_handler), 100);
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
