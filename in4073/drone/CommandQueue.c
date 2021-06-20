#include "CommandQueue.h"

#include <stddef.h>
#include <stdio.h>
//authored by Nathan
void CommandQueue_init(struct CommandQueue *queue)
{
	queue->first = 0;
	queue->last = COMMAND_QUEUE_SIZE - 1;
	queue->count = 0;
}
//authored by Nathan
bool CommandQueue_push(struct CommandQueue *queue, struct Command *command)
{
    if (queue->count < COMMAND_QUEUE_SIZE)
    {
        queue->last = (queue->last + 1) % COMMAND_QUEUE_SIZE;
        queue->commands[queue->last] = command;
        queue->count += 1;
        return true;
    }

    return false;
}
//authored by Nathan
struct Command *CommandQueue_pop(struct CommandQueue *queue)
{
    if (queue->count > 0)
    {
        struct Command *first_item = queue->commands[queue->first];
        queue->first = (queue->first + 1) % COMMAND_QUEUE_SIZE;
        queue->count -= 1;

        return first_item;
    }

    return NULL;
}
//authored by Nathan
bool CommandQueue_empty(struct CommandQueue *queue)
{
    return queue->count == 0;
}
