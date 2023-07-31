#include <math.h>
#include "usart.h"

#ifdef RTOS

	#include "FreeRTOS.h"
	#include "task.h"
#elif
	#include "delay.h"

#endif


void usart_init(USART_TypeDef *usart, uint32_t baudrate)
{
	/* usart clock enable */
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
	/* usart pins */
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER2, 0b10 << GPIO_MODER_MODER2_Pos);
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER3, 0b10 << GPIO_MODER_MODER3_Pos);
	MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED2, 0b01 << GPIO_OSPEEDR_OSPEED2_Pos);
	MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED3, 0b01 << GPIO_OSPEEDR_OSPEED3_Pos);
	MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFSEL2, 0b0111 << GPIO_AFRL_AFSEL2_Pos);
	MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFSEL3, 0b0111 << GPIO_AFRL_AFSEL3_Pos);
	/* Turn off usart */
	CLEAR_BIT(usart->CR1, USART_CR1_UE);

	/* Word len */
	CLEAR_BIT(usart->CR1, USART_CR1_M);
	/* Stop bits */
	WRITE_REG(usart->CR2, 0b00 << USART_CR2_STOP_Pos);
	/* Set Speed */
	uint32_t input_clock_hz = 42000000;
	float bbr_val = (float)input_clock_hz / baudrate / 16;
	uint16_t mantissa = (uint16_t)bbr_val;
	float fraction = ceil((bbr_val - mantissa) * 16);
	MODIFY_REG(usart->BRR, USART_BRR_DIV_Mantissa, mantissa << USART_BRR_DIV_Mantissa_Pos);
	MODIFY_REG(usart->BRR, USART_BRR_DIV_Fraction, (uint8_t)fraction << USART_BRR_DIV_Fraction_Pos);
	/* Parity control */
	CLEAR_BIT(usart->CR1, USART_CR1_PCE);
	/* Transmit Receive enable */
	SET_BIT(usart->CR1, USART_CR1_TE);
	SET_BIT(usart->CR1, USART_CR1_RE);
	/* DMA for transmit */
	SET_BIT(usart->CR3, USART_CR3_DMAT);

	/* Turn on usart */
	SET_BIT(usart->CR1, USART_CR1_UE);
}


bool usart_transmit(USART_TypeDef *usart, uint8_t *buf, uint16_t len, uint32_t timeout)
{
#ifdef RTOS
	uint32_t current_tick = xTaskGetTickCount();
#elif
	uint32_t current_tick = get_tick();
#endif
	/*If previous transmition has completed*/
	if (READ_BIT(usart->SR, USART_SR_TC))
	{
		for (uint16_t i = 0; i < len; i++)
		{
			/*Wait till prev byte will be transfered to shift register*/
			while (READ_BIT(usart->SR, USART_SR_TXE ) != USART_SR_TXE)
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
			usart->DR = buf[i];
		}
		SET_BIT(usart->SR, USART_SR_TC);
	}

	return true;
}






