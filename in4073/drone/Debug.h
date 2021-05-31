//
// Created by nathan on 27-05-21.
//

#ifndef QUADCOPTER_FCB_DEBUG_H
#define QUADCOPTER_FCB_DEBUG_H

#include "CommandHandler.h"

#include <stddef.h>

extern struct CommandHandler **global_channel;

void DEBUG_SET_CHANNEL(struct CommandHandler **debug_channel);
void DEBUG(const char *format, ...);

#endif //QUADCOPTER_FCB_DEBUG_H
