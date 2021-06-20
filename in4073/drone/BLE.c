//
// Created by nathan on 28-04-21.
//

#include "BLE.h"
#include <stdlib.h>
#include <stdio.h>
#include "../utils/quad_ble.h"

#include "Debug.h"
//authored by Nathan
struct Comms *BLE_create()
{
    struct Comms *result = (struct Comms *)malloc(sizeof(struct Comms));

    if (result)
    {
        Comms_init(result);
        quad_ble_init();

        result->loop = LoopHandler_init_controlblock(BLE_loop);
    }

    return result;
}
//authored by Nathan
void BLE_loop(void *context, uint32_t delta_us)
{
    struct Comms *self = (struct Comms *)context;

    while (ble_rx_queue.count)
    {
//        __disable_irq();
        uint8_t b = dequeue(&ble_rx_queue);
//        __enable_irq();

        Comms_decoder_append_byte(self, b);
    }

    while(!CommandQueue_empty(&self->send_queue))
    {
        struct Command *command = CommandQueue_pop(&self->send_queue);
        struct EncodedCommand encoded = Command_encode(command);

        for (uint8_t i = 0; i < encoded.size; i += 1)
        {
            enqueue(&ble_tx_queue, encoded.data[i]);
        }

        free(encoded.data);
        Command_destroy(command);
    }

    quad_ble_send();
}
//authored by Nathan
void BLE_destroy(struct Comms *self)
{
    if (self)
    {
        free(self);
    }
}