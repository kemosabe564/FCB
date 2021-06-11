//
// Created by nathan on 26-04-21.
//

#ifndef QUADCOPTER_FCB_COMMS_H
#define QUADCOPTER_FCB_COMMS_H

#include <stdbool.h>
#include <stdint.h>
#include "Command.h"
#include "LoopHandler.h"
#include "CommandQueue.h"

#define COMMS_DECODER_SIZE 20

struct Comms {
    struct LoopHandlerControlBlock loop;

    // serial command decoder members
    uint8_t buffer[COMMS_DECODER_SIZE];
    uint16_t char_id;
    uint8_t data_len;

    struct CommandQueue send_queue;
    struct CommandQueue receive_queue;
};

void Comms_init(struct Comms *self);
bool Comms_decoder_append_byte(struct Comms *self, uint8_t byte);

#endif //QUADCOPTER_FCB_COMMS_H
