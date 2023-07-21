#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include <stdint.h>

#include "main.h"


void delay_ms_init(uint32_t sys_core_clock);
void delay_ms(uint32_t ms);
uint32_t get_tick(void);

#endif /* INC_DELAY_H_ */
