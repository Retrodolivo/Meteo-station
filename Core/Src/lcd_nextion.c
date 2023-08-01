#include <string.h>
#include <stdio.h>
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
	for (uint16_t i = 0; i < len; i++)
	{
		lcd.txbuf[i] = string[i];
	}
	for (uint8_t i = 0; i < TERM_SYMB_AMOUNT; i++)
	{
		lcd.txbuf[len + i] = lcd.msg_terminator[i];
	}
	lcd_write_cmplt_flag = false;
	dma1_to_periph_start((uint32_t)lcd.txbuf, (uint32_t)&lcd.uart->DR, len + TERM_SYMB_AMOUNT);
	lcd.last_msg_len = len + TERM_SYMB_AMOUNT;

	return true;
}


bool lcd_set_page(char *page, uint32_t timeout)
{
	bool ret = false;

	char temp[100];
	uint16_t len = sprintf(temp, "page %s", page);
	ret = lcd_write((uint8_t *)temp, len, timeout);

	return ret;
}

bool lcd_set_txt_color(char *label, char *color, uint32_t timeout)
{
	bool ret = false;

	char temp[100];
	uint16_t len = sprintf(temp, "%s.pco=%s", label, color);
	ret = lcd_write((uint8_t *)temp, len, timeout);

	return ret;
}

bool lcd_set_txt(char *label, char *txt, uint32_t timeout)
{
	bool ret = false;

	char temp[100];
	uint16_t len = sprintf(temp, "%s.txt=\"%s\"", label, txt);
	ret = lcd_write((uint8_t *)temp, len, timeout);

	return ret;
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
