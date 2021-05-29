//
// Created by nathan on 27-05-21.
//

#include "Debug.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Command.h"
#include "CommandHandler.h"

struct CommandHandler **global_channel = NULL;

void DEBUG_SET_CHANNEL(struct CommandHandler **debug_channel)
{
    global_channel = debug_channel;
}

void DEBUG(const char *format, ...)
{
    if (*global_channel == NULL)
        return;

    va_list args;
    va_start(args, format);
    uint16_t size = vsprintf(NULL, format, args);

    char *message = (char *)malloc(size * sizeof(char));
    vsprintf(message, format, args);

    uint16_t msg_pos = 0;
    uint16_t msg_size = 0;

    while (size)
    {
        msg_size = (size > 17) ? 17 : size;

        CommandHandler_send_command(*global_channel, Command_make_debug_n(&message[msg_pos], msg_size));

        msg_pos += msg_size;
        size -= msg_size;
    }

    free(message);
}
