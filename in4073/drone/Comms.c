//
// Created by nathan on 26-04-21.
//

#include "Comms.h"

#include <stddef.h>
#include <stdio.h>
//authored by Nathan
void Comms_init(struct Comms *self)
{
    if (self)
    {
        CommandQueue_init(&self->send_queue);
        CommandQueue_init(&self->receive_queue);

        for (int i = 0; i < COMMS_DECODER_SIZE; i += 1)
            self->buffer[i] = 0;

        self->char_id = 0;
        self->data_len = 0;
    }
}
//authored by Nathan
bool Comms_decoder_append_byte(struct Comms *self, uint8_t byte)
{
    self->buffer[self->char_id] = byte;

    if (self->char_id == 0) // Header byte
    {
        self->data_len = Command_data_len(byte);
        self->char_id++;
    }
    else // Remaining bytes
    {
        if (self->data_len == 0) // last data byte has been read, now CRC byte
        {
            struct Command *cmd = Command_decode(self->buffer);
            if (!CommandQueue_push(&self->receive_queue, cmd))
            {
                Command_destroy(cmd);
            }
            else
            {
                self->char_id = 0;
                return true;
            }
        }
        else // Data bytes
        {
            self->data_len--;
            self->char_id++;
        }
    }

    return false;
}