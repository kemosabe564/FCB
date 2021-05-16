//
// Created by nathan on 28-04-21.
//

#include "Serial.h"
#include <stdlib.h>
#include <stdio.h>
#include "../hal/uart.h"

struct Comms *Serial_create(int32_t baud_rate)
{
    struct Comms *result = (struct Comms *)malloc(sizeof(struct Comms));

    if (result)
    {
        uart_init();

        result->first = 0;
        result->last = COMMS_QUEUE_LEN - 1;
        result->count = 0;

        result->loop = LoopHandler_init_controlblock(Serial_loop);
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
        uint8_t b = dequeue(&rx_queue);
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
                Comms_enqueue_command(self, Command_decode(raw_str));
                char_id = 0;
            }
            else // Data bytes
            {
                data_len--;
                char_id++;
            }
        }
    }
}

void Serial_destroy(struct Comms *self)
{
    if (self)
    {
        free(self);
    }
}