#include <string.h>
#include "lcd_nextion.h"
#include "dma.h"

USART_TypeDef *lcd_uart;
uint8_t terminator[3] = {0xFF, 0xFF, 0xFF};

void lcd_init(USART_TypeDef *uart)
{
	lcd_uart = uart;
}

bool lcd_write(uint8_t *buf, uint16_t len)
{
	bool ret = false;
//	strcat((char *)buf, (char *)terminator);
	if (dma1_to_periph_start((uint32_t)buf, (uint32_t)&lcd_uart->DR, len))
	{
		ret = true;
	}
	return ret;
}

void DMA1_Stream6_IRQHandler(void)
{
	// DMA transfer complete.
	if (READ_BIT(DMA1->HISR, DMA_HISR_TCIF6))
	{
		WRITE_REG(DMA1->HIFCR , DMA_HIFCR_CTCIF6);
	}
	// DMA transfer error.
	if (READ_BIT(DMA1->HISR ,DMA_HISR_TEIF6))
	{
		WRITE_REG(DMA1->HIFCR , DMA_HIFCR_CTEIF6);
	}
}
