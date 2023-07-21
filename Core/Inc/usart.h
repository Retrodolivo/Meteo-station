#ifndef INC_USART_H_
#define INC_USART_H_

#include <stdbool.h>

#include "main.h"

void usart_init(USART_TypeDef *uart, uint32_t baudrate);
bool uart_transmit(USART_TypeDef *uart, uint8_t *buf, uint16_t len, uint32_t timeout);

#endif /* INC_USART_H_ */
