#ifndef INC_USART_H_
#define INC_USART_H_

#include <stdbool.h>

#include "main.h"

/**
 * UART initializing
 * @param uart Pointer to usart structure which is going to be initialized
 * @param baudrate uint32_t val which specifies usart speed. Has been tested with standard values [1200, 4800, 9600, ... 115200, etc]
 */
void usart_init(USART_TypeDef *usart, uint32_t baudrate);

/**
 * usart_transmit() send data through usart
 * non-blocking function. Unblocks after timeout
 * @param usart Pointer to usart structure port which is going to be used
 * @param txbuf uint8_t array that contains data that would be sent by usart
 * @param txlen uint16_t var that contains amount of elems in @txbuf
 * @param timeout uint32_t var that specifies time in milliseconds which func waiting for the flags
 * @return return true if transfer had completed before timeout struck
 */
bool usart_transmit(USART_TypeDef *usart, uint8_t *txbuf, uint16_t txlen, uint32_t timeout);

#endif /* INC_USART_H_ */
