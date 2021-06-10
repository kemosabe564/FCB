//
// Created by nathan on 28-04-21.
//

#include "Serial.h"
#include <stdlib.h>
#include <stdio.h>
#include "Debug.h"
#include "../hal/uart.h"

#include "nrf_gpio.h"
#include "nrf_delay.h"

struct Comms *Serial_create(int32_t baud_rate)
{
    struct Comms *result = (struct Comms *)malloc(sizeof(struct Comms));

    if (result)
    {
        uart_init();

        CommandQueue_init(&result->send_queue);
        CommandQueue_init(&result->receive_queue);

        result->loop = LoopHandler_init_controlblock(Serial_loop);

        // send protocol enable message to terminal
        for (uint8_t i = 0; i < 10; i += 1)
            uart_put(0xff);

    }

    return result;
}

void Serial_loop(void *context, uint32_t delta_us)
{
    struct Comms *self = (struct Comms *)context;

    // These static values should be placed somewhere else
    // static so they keep there value upon next loop iteration
    static uint8_t raw_str[256] = { 0 };
    static uint16_t char_id = 0;
    static uint8_t data_len = 0;

    while (rx_queue.count)
    {
        __disable_irq();
        uint8_t b = dequeue(&rx_queue);
        __enable_irq();
        raw_str[char_id] = b;

        if (char_id == 0) // Header byte
        {
            data_len = Command_data_len(b);
            char_id++;
        }
        else // Remaining bytes
        {
            if (data_len == 0) // CRC byte
            {
                struct Command *cmd = Command_decode(raw_str);
                if (!CommandQueue_push(&self->receive_queue, cmd))
                {
                    Command_destroy(cmd);
                }
                char_id = 0;
            }
            else // Data bytes
            {
                data_len--;
                char_id++;
            }
        }
    }

    while(!CommandQueue_empty(&self->send_queue))
    {
        struct Command *command = CommandQueue_pop(&self->send_queue);
        struct EncodedCommand encoded = Command_encode(command);

        uart_put_n(encoded.data, encoded.size);

        free(encoded.data);
        Command_destroy(command);
    }
}

void Serial_destroy(struct Comms *self)
{
    if (self)
    {
        free(self);
    }
}