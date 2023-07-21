#ifndef INC_DMA_H_
#define INC_DMA_H_

#include <stdbool.h>
#include "main.h"

void dma1_init(void);
bool dma1_to_periph_start(uint32_t src_addr, uint32_t dst_addr, uint16_t data_size);

#endif /* INC_DMA_H_ */
