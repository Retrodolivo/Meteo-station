#include <math.h>
#include "usart.h"
#include "delay.h"

#define RTOS

void usart_init(USART_TypeDef *uart, uint32_t baudrate)
{
	/* uart clock enable */
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
	/* uart pins */
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER2, 0b10 << GPIO_MODER_MODER2_Pos);
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER3, 0b10 << GPIO_MODER_MODER3_Pos);
	MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED2, 0b01 << GPIO_OSPEEDR_OSPEED2_Pos);
	MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED3, 0b01 << GPIO_OSPEEDR_OSPEED3_Pos);
	MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFSEL2, 0b0111 << GPIO_AFRL_AFSEL2_Pos);
	MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFSEL3, 0b0111 << GPIO_AFRL_AFSEL3_Pos);
	/* Turn off uart */
	CLEAR_BIT(uart->CR1, USART_CR1_UE);

	/* Word len */
	CLEAR_BIT(uart->CR1, USART_CR1_M);
	/* Stop bits */
	WRITE_REG(uart->CR2, 0b00 << USART_CR2_STOP_Pos);
	/* Set Speed */
	uint32_t input_clock_hz = 42000000;
	float bbr_val = (float)input_clock_hz / baudrate / 16;
	uint16_t mantissa = (uint16_t)bbr_val;
	float fraction = ceil((bbr_val - mantissa) * 16);
	MODIFY_REG(uart->BRR, USART_BRR_DIV_Mantissa, mantissa << USART_BRR_DIV_Mantissa_Pos);
	MODIFY_REG(uart->BRR, USART_BRR_DIV_Fraction, (uint8_t)fraction << USART_BRR_DIV_Fraction_Pos);
	/* Parity control */
	CLEAR_BIT(uart->CR1, USART_CR1_PCE);
	/* Transmit Receive enable */
	SET_BIT(uart->CR1, USART_CR1_TE);
	SET_BIT(uart->CR1, USART_CR1_RE);
	/* DMA for transmit */
	SET_BIT(uart->CR3, USART_CR3_DMAT);

	/* Turn on uart */
	SET_BIT(uart->CR1, USART_CR1_UE);
}


#ifndef RTOS
bool uart_transmit(USART_TypeDef *uart, uint8_t *buf, uint16_t len, uint32_t timeout)
{
	uint32_t current_tick = get_tick();

	/*If previous transmition has completed*/
	if (READ_BIT(uart->SR, USART_SR_TC))
	{
		for (uint16_t i = 0; i < len; i++)
		{
			/*Wait till prev byte will be transfered to shift register*/
			while (READ_BIT(uart->SR, USART_SR_TXE ) != USART_SR_TXE)
			{
				if ((get_tick() - current_tick) > timeout)
				{
					return false;
				}
			}
			uart->DR = buf[i];
		}
		SET_BIT(uart->SR, USART_SR_TC);
	}

	return true;
}
#endif






