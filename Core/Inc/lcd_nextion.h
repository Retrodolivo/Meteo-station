#ifndef INC_LCD_NEXTION_H_
#define INC_LCD_NEXTION_H_

#include <stdbool.h>

#include "main.h"
#include "usart.h"

#define TERM_SYMB			0xFE
#define TERM_SYMB_AMOUNT	3

typedef struct
{
	USART_TypeDef *uart;
	uint8_t *txbuf;
	uint8_t msg_terminator[TERM_SYMB_AMOUNT];
	uint16_t last_msg_len;
} LCD_st;

void lcd_init(USART_TypeDef *uart, uint8_t *txbuf);
bool lcd_write(uint8_t *string, uint16_t len, uint32_t timeout);


#endif /* INC_LCD_NEXTION_H_ */
