#ifndef INC_LCD_NEXTION_H_
#define INC_LCD_NEXTION_H_

#include <stdbool.h>

#include "main.h"
#include "usart.h"

void lcd_init(USART_TypeDef *uart);
bool lcd_write(uint8_t *buf, uint16_t len);


#endif /* INC_LCD_NEXTION_H_ */
