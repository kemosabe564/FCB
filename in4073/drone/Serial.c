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
//authored by Nathan
struct Comms *Serial_create(int32_t baud_rate)
{
    struct Comms *result = (struct Comms *)malloc(sizeof(struct Comms));

    if (result)
    {
        Comms_init(result);
        uart_init();

        result->loop = LoopHandler_init_controlblock(Serial_loop);

        // send protocol enable message to terminal
        for (uint8_t i = 0; i < 10; i += 1)
            uart_put(0xff);

    }

    return result;
}
//authored by Nathan
void Serial_loop(void *context, uint32_t delta_us)
{
    struct Comms *self = (struct Comms *)context;

    while (rx_queue.count)
    {
        __disable_irq();
        uint8_t b = dequeue(&rx_queue);
        __enable_irq();

        Comms_decoder_append_byte(self, b);
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
//authored by Nathan
void Serial_destroy(struct Comms *self)
{
    if (self)
    {
        free(self);
    }
}