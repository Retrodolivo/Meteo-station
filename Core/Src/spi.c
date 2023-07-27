#include "spi.h"
#include "delay.h"

void spi_init(SPI_TypeDef *spi)
{
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_SPI2EN);


	while (READ_BIT(spi->SR, SPI_SR_BSY));
	CLEAR_BIT(spi->CR1, SPI_CR1_SPE);

	MODIFY_REG(spi->CR1, SPI_CR1_BR, 0b101 << SPI_CR1_BR_Pos);
	/* SPI MODE */
	CLEAR_BIT(spi->CR1, SPI_CR1_CPOL);
	CLEAR_BIT(spi->CR1, SPI_CR1_CPHA);
	/* Data width */
	CLEAR_BIT(spi->CR1, SPI_CR1_DFF);
	/*RX TX modes*/
	CLEAR_BIT(spi->CR1, SPI_CR1_RXONLY);
	/* First bit*/
	CLEAR_BIT(spi->CR1, SPI_CR1_LSBFIRST);
	/* NSS pin */
	SET_BIT(spi->CR1, SPI_CR1_SSM);
	SET_BIT(spi->CR1, SPI_CR1_SSI);
	/*Master/Slave Mode*/
	SET_BIT(spi->CR1, SPI_CR1_MSTR);

	/* Interrupts */
	SET_BIT(spi->CR2, SPI_CR2_TXEIE);
	SET_BIT(spi->CR2, SPI_CR2_RXNEIE);
	SET_BIT(spi->CR2, SPI_CR2_ERRIE);

	/* gpios*/
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);
	/* CLK */
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER13, 0b10 << GPIO_MODER_MODER13_Pos);
	CLEAR_BIT(GPIOB->OTYPER, GPIO_OTYPER_OT13);
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED13, 0b10 << GPIO_OSPEEDR_OSPEED13_Pos);
	MODIFY_REG(GPIOB->AFR[1], GPIO_AFRH_AFSEL13, 0b0101 << GPIO_AFRH_AFSEL13_Pos);
	/* MISO */
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER14, 0b10 << GPIO_MODER_MODER14_Pos);
	CLEAR_BIT(GPIOB->OTYPER, GPIO_OTYPER_OT14);
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED14, 0b10 << GPIO_OSPEEDR_OSPEED14_Pos);
	MODIFY_REG(GPIOB->AFR[1], GPIO_AFRH_AFSEL14, 0b0101 << GPIO_AFRH_AFSEL14_Pos);
	/* MOSI */
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER15, 0b10 << GPIO_MODER_MODER15_Pos);
	CLEAR_BIT(GPIOB->OTYPER, GPIO_OTYPER_OT15);
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED15, 0b10 << GPIO_OSPEEDR_OSPEED15_Pos);
	MODIFY_REG(GPIOB->AFR[1], GPIO_AFRH_AFSEL15, 0b0101 << GPIO_AFRH_AFSEL15_Pos);

	SET_BIT(spi->CR1, SPI_CR1_SPE);

//	NVIC_SetPriority(SPI2_IRQn, 2);
//	NVIC_EnableIRQ(SPI2_IRQn);

}

bool spi_transmit(SPI_TypeDef *spi, uint8_t *txbuf, uint16_t txlen, uint32_t timeout)
{
	uint32_t current_tick = get_tick();

	for (uint8_t i = 0; i < txlen; i++)
	{
		while (!(READ_BIT(spi->SR, SPI_SR_TXE)))
		{
			if (get_tick() - current_tick > timeout)
			{
				return false;
			}
		}
		spi->DR = txbuf[i];
		while (!(READ_BIT(spi->SR, SPI_SR_RXNE)))
		{
			if (get_tick() - current_tick > timeout)
			{
				return false;
			}
		}
		/* Reset RXNE flag */
		uint8_t temp = spi->DR;

	}

	return true;
}

bool spi_receive(SPI_TypeDef *spi, uint8_t *rxbuf, uint16_t rxlen, uint32_t timeout)
{
	uint32_t current_tick = get_tick();

	for (uint8_t i = 0; i < rxlen; i++)
	{
		spi->DR = 0;
		while (!(READ_BIT(spi->SR, SPI_SR_RXNE)))
		{
			if (get_tick() - current_tick > timeout)
			{
				return false;
			}
		}
		rxbuf[i] = spi->DR;
	}

	return true;
}




