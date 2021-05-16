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
#include "hal/adc.h"
#include "hal/barometer.h"
#include "hal/gpio.h"
#include "hal/spi_flash.h"
#include "hal/timers.h"
#include "hal/twi.h"
#include "hal/uart.h"
#include "control.h"
#include "mpu6050/mpu6050.h"
#include "utils/quad_ble.h"

#include "drone/LoopHandler.h"
#include "drone/CommandHandler.h"
#include "drone/RotorMap.h"
#include "drone/Rotor.h"
#include "drone/Serial.h"
#include "drone/IMU.h"
#include "drone/FlightController.h"

bool demo_done;

#define COMM_SERIAL 0
#define COMM_BLE    1

struct FlightController *fc = NULL;

void command_handler_function(struct Command *command)
{
    switch (command->type)
    {
        case SetOrQueryMode:
        {
            printf("SetOrQueryMode \n");
            enum FlightControllerMode *mode = (enum FlightControllerMode *)command->data;

            FlightController_change_mode(fc, *mode);
        }
            break;
        case SetControl: {
            struct CommandControlData *data = (struct CommandControlData *)command->data;

            printf("Yaw %d, Pitch %d, Roll %d \n", data->yaw_rate, data->pitch_rate, data->roll_rate);
        }
            break;
        case Invalid:
            printf("Invalid command");
            break;
        default:
            break;
    }
}

void heartbeat(void *context, uint32_t delta_us)
{
    static int num = 0;
    printf("Heartbeat %d \n", num++);
}

struct LoopHandlerControlBlock heartbeat_cb = {
    .func = heartbeat
};

int main(void)
{
    bool running = true;


    uart_init();
    gpio_init();
    timers_init();
    adc_init();
    twi_init();
    imu_init(true, 100);
    baro_init();
    spi_flash_init();
    quad_ble_init();


    struct LoopHandler *lh = LoopHandler_create();

    struct RotorMap *r_map = RotorMap_create(0, 10000);

    struct Rotor *r1 = Rotor_create(r_map, 0, -15,  15);
    struct Rotor *r2 = Rotor_create(r_map, 1,  15,  15);
    struct Rotor *r3 = Rotor_create(r_map, 2, -15, -15);
    struct Rotor *r4 = Rotor_create(r_map, 3, -15,  15);

    struct IMU *imu = IMU_create();

    fc = FlightController_create(imu, (struct Rotor *[]){ r1, r2, r3, r4 }, 4);

//    struct Comms ble_comms BLE_init();
    struct Comms *serial_comms = Serial_create(115200);

    struct CommandHandler *comm_handler = CommandHandler_create(COMM_SERIAL, command_handler_function);

    CommandHandler_add_comms(comm_handler, COMM_SERIAL, serial_comms);
//    CommandHandler_add_comms(comm_handler, ble_comms, COMM_BLE);




	while (running)
	{
        LoopHandler_loop(lh, LH_LINK(fc), LH_HZ_TO_PERIOD(10));

        LoopHandler_loop(lh, LH_LINK(r1), LH_HZ_TO_PERIOD(100));
        LoopHandler_loop(lh, LH_LINK(r2), LH_HZ_TO_PERIOD(100));
        LoopHandler_loop(lh, LH_LINK(r3), LH_HZ_TO_PERIOD(100));
        LoopHandler_loop(lh, LH_LINK(r4), LH_HZ_TO_PERIOD(100));


        LoopHandler_loop(lh, LH_LINK(serial_comms), LH_HZ_TO_PERIOD(20));
//        LoopHandler_loop(lh, LH_LINK(ble_comms), LH_HZ_TO_PERIOD(50));

        LoopHandler_loop(lh, LH_LINK(comm_handler), LH_HZ_TO_PERIOD(100));

        LoopHandler_loop(lh, &heartbeat_cb, (void *) NULL, 5000000);
	}

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
