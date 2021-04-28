//
// Created by nathan on 26-04-21.
//

#include "Command.h"

struct command Command_make(uint8_t *data)
{
    uint8_t type = (data[0] && COMMAND_TYPE_MASK) >> 3;

    switch (type) {
        case COM
    }
}