#include <string.h>
#include "lcd_nextion.h"
#include "dma.h"

#ifdef RTOS

	#include "FreeRTOS.h"
	#include "task.h"
#elif
	#include "delay.h"

#endif


static LCD_st lcd;

static bool lcd_write_cmplt_flag = true;

static void lcd_buf_flush(void);

void lcd_init(USART_TypeDef *uart, uint8_t *txbuf)
{
	lcd.uart = uart;
	lcd.txbuf = txbuf;
	memset(lcd.msg_terminator, TERM_SYMB, NUM_ELEMS(lcd.msg_terminator));
}


bool lcd_write(uint8_t *string, uint16_t len, uint32_t timeout)
{
#ifdef RTOS
	uint32_t current_tick = xTaskGetTickCount();
#elif
	uint32_t current_tick = get_tick();

#endif
	for (uint16_t i = 0; i < len; i++)
	{
		lcd.txbuf[i] = string[i];
	}
	for (uint8_t i = 0; i < TERM_SYMB_AMOUNT; i++)
	{
		lcd.txbuf[i + len] = lcd.msg_terminator[i];
	}
	while (!lcd_write_cmplt_flag)
	{
#ifdef RTOS
		if ((xTaskGetTickCount() - current_tick) > timeout)
		{
			return false;
		}
#elif
		if ((get_tick() - current_tick) > timeout)
		{
			return false;
		}
#endif
	}
	lcd_write_cmplt_flag = false;
	dma1_to_periph_start((uint32_t)lcd.txbuf, (uint32_t)&lcd.uart->DR, len + TERM_SYMB_AMOUNT);
	lcd.last_msg_len = len + TERM_SYMB_AMOUNT;

	return true;
}

static void lcd_buf_flush(void)
{
	memset(lcd.txbuf, 0, lcd.last_msg_len);
}

void DMA1_Stream6_IRQHandler(void)
{
	// DMA transfer complete.
	if (READ_BIT(DMA1->HISR, DMA_HISR_TCIF6))
	{
		lcd_write_cmplt_flag = true;
		WRITE_REG(DMA1->HIFCR , DMA_HIFCR_CTCIF6);
	}
	// DMA transfer error.
	if (READ_BIT(DMA1->HISR ,DMA_HISR_TEIF6))
	{
		WRITE_REG(DMA1->HIFCR , DMA_HIFCR_CTEIF6);
	}

}
