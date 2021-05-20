#ifndef UART_H_
#define UART_H_

#include "../utils/queue.h"

extern Queue rx_queue;
extern Queue tx_queue;

void uart_init(void);
void uart_put(uint8_t byte);
void uart_put_n(uint8_t *bytes, uint16_t n);

#endif /* HAL_UART_H_ */
