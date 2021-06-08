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
#include <stdlib.h>

#include "drone/LoopHandler.h"
#include "drone/CommandHandler.h"
#include "drone/RotorMap.h"
#include "drone/Rotor.h"
#include "drone/Serial.h"
#include "drone/IMU.h"
#include "drone/FlightController.h"
#include "drone/Debug.h"
#include "drone/Telemetry.h"

bool demo_done;

#define COMM_SERIAL 0
#define COMM_BLE    1

struct FlightController *fc = NULL;
struct CommandHandler *ch = NULL;
struct Telemetry *telemetry = NULL;

void changed_mode_handler(struct FlightController *self, enum FlightControllerMode new_mode, enum FlightControllerMode old_mode)
{
    CommandHandler_send_command(ch, Command_make_current_mode((uint8_t) new_mode));

    if (new_mode == Safe)
    {
        nrf_gpio_pin_set(RED);
        nrf_gpio_pin_set(YELLOW);
        nrf_gpio_pin_clear(GREEN);
        nrf_gpio_pin_set(BLUE);
    }
    else if (new_mode == Panic)
    {
        nrf_gpio_pin_clear(RED);
        nrf_gpio_pin_set(YELLOW);
        nrf_gpio_pin_set(GREEN);
        nrf_gpio_pin_set(BLUE);
    }
    else if (new_mode == Manual)
    {
        nrf_gpio_pin_set(RED);
        nrf_gpio_pin_clear(YELLOW);
        nrf_gpio_pin_set(GREEN);
        nrf_gpio_pin_set(BLUE);
    }
}

void heartbeat_lost()
{
    FlightController_change_mode(fc, Panic);
    DEBUG(0, "Heartbeat lost :(");
}

void command_handler_function(struct Command *command)
{
    switch (command->type)
    {
        case SetOrQueryMode:
        {
            uint8_t *mode = (uint8_t *)command->data;

            if (*mode > 0)
            {
                FlightController_change_mode(fc, *mode);
            }
            else
            {
                CommandHandler_send_command(ch, Command_make_current_mode((uint8_t) fc->mode));
            }
        }
            break;
        case SetControl: {
            struct CommandControlData *data = (struct CommandControlData *)command->data;

            if (data)
            {
                FlightController_set_controls(fc, data->yaw_rate, data->pitch_rate, data->roll_rate, data->climb_rate);
            }
        }
            break;
        case SetParam: {
            struct CommandParamsData *data = (struct CommandParamsData *) command->data;

            if (data)
            {
                switch (data->id)
                {
                case 0:
                case 1:
                case 2: // P values
                    FlightController_set_params(fc, data->id , data->value);
                    break;
                case 3:
                    Telemetry_set_enabled(telemetry, data->value == 1);
                    break;

                }
            }
        }
            break;
        case Invalid:
            printf("Invalid command");
            break;
        default:
            break;
    }
}

int main(void)
{
    bool running = true;

    DEBUG_SET_CHANNEL(&ch);

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

    struct IMU *imu = IMU_create(true, 100);


//    struct Comms ble_comms BLE_init();
    struct Comms *serial_comms = Serial_create(115200);

    ch = CommandHandler_create(COMM_SERIAL, command_handler_function);

    CommandHandler_add_comms(ch, COMM_SERIAL, serial_comms);
//    CommandHandler_add_comms(comm_handler, ble_comms, COMM_BLE);
    CommandHandler_set_on_heartbeat_lost(ch, 5000 * 1000 /* in us */, heartbeat_lost);

    fc = FlightController_create(imu, (struct Rotor *[]){ r1, r2, r3, r4 }, 4, ch);
    FlightController_set_on_change_mode(fc, changed_mode_handler);

    telemetry = Telemetry_create(ch, imu, (struct Rotor *[]){ r1, r2, r3, r4 }, 4);

	while (running)
	{
        LoopHandler_loop(lh, LH_LINK(fc), LH_HZ_TO_PERIOD(200));

        LoopHandler_loop(lh, LH_LINK(r1), 0);
        LoopHandler_loop(lh, LH_LINK(r2), 0);
        LoopHandler_loop(lh, LH_LINK(r3), 0);
        LoopHandler_loop(lh, LH_LINK(r4), 0);

        LoopHandler_loop(lh, LH_LINK(imu), LH_HZ_TO_PERIOD(100));

        LoopHandler_loop(lh, LH_LINK(serial_comms), 0);
//        LoopHandler_loop(lh, LH_LINK(ble_comms), LH_HZ_TO_PERIOD(50));

        LoopHandler_loop(lh, LH_LINK(ch), 0);

        LoopHandler_loop(lh, LH_LINK(telemetry), LH_HZ_TO_PERIOD(1));
	}

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
