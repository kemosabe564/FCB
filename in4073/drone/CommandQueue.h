#ifndef COMMAND_QUEUE_H_
#define COMMAND_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>

#include "Command.h"
#define COMMAND_QUEUE_SIZE 32

struct CommandQueue
{
	struct Command *commands[COMMAND_QUEUE_SIZE];
	uint16_t first;
	uint16_t last;
	uint16_t count;
};

void CommandQueue_init(struct CommandQueue *queue);
bool CommandQueue_push(struct CommandQueue *queue, struct Command *command);
bool CommandQueue_empty(struct CommandQueue *queue);
struct Command *CommandQueue_pop(struct CommandQueue *queue);

#endif /* QUEUE_H_ */
