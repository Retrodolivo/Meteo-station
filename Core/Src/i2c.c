#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"


static void sw_reset(void);
static void send_dev_addr(I2C_TypeDef *i2c, uint8_t dev_addr, Action_et action);
static bool send_data_byte(I2C_TypeDef *i2c, uint8_t *buf);
static bool read_data_bytes(I2C_TypeDef *i2c, uint8_t *buf, uint16_t rx_size, uint32_t current_tick, uint32_t timeout);
static bool mem_write_data(I2C_TypeDef *i2c, uint8_t dev_addr, uint16_t reg_addr, Reg_mem_size_et e_reg_size, uint8_t *buf, uint16_t tx_size, uint32_t current_tick, uint32_t timeout);
static bool mem_read_data(I2C_TypeDef *i2c, uint8_t dev_addr, uint16_t reg_addr, Reg_mem_size_et e_reg_size, uint8_t *buf, uint16_t rx_size, uint32_t current_tick, uint32_t timeout);


void i2c_init(I2C_TypeDef *i2c)
{
	uint8_t input_clock_mhz = 42;

	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C3EN);

	/* GPIOs */
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN);

	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER8, 0b10<<GPIO_MODER_MODER8_Pos);
	MODIFY_REG(GPIOA->OTYPER, GPIO_OTYPER_OT_8, 0b01<<GPIO_OTYPER_OT8_Pos);
	MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED8, 0b10<<GPIO_OSPEEDR_OSPEED8_Pos);
	MODIFY_REG(GPIOA->PUPDR, GPIO_PUPDR_PUPDR8, 0b00<<GPIO_PUPDR_PUPD8_Pos);
	MODIFY_REG(GPIOA->AFR[1], GPIO_AFRH_AFSEL8, 4<<GPIO_AFRH_AFSEL8_Pos);

	MODIFY_REG(GPIOC->MODER, GPIO_MODER_MODER9, 0b10<<GPIO_MODER_MODER9_Pos);
	MODIFY_REG(GPIOC->OTYPER, GPIO_OTYPER_OT_9, 0b01<<GPIO_OTYPER_OT9_Pos);
	MODIFY_REG(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED9, 0b10<<GPIO_OSPEEDR_OSPEED9_Pos);
	MODIFY_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPDR9, 0b00<<GPIO_PUPDR_PUPD9_Pos);
	MODIFY_REG(GPIOC->AFR[1], GPIO_AFRH_AFSEL9, 4<<GPIO_AFRH_AFSEL9_Pos);


	CLEAR_BIT(i2c->CR1, I2C_CR1_PE);

	sw_reset();

	MODIFY_REG(i2c->CR2, I2C_CR2_FREQ, input_clock_mhz<<I2C_CR2_FREQ_Pos);
	CLEAR_BIT(i2c->CCR, I2C_CCR_FS);
	MODIFY_REG(i2c->CCR, I2C_CCR_CCR, 210<<I2C_CCR_CCR_Pos);
	/* Max T rise = 1us */
	MODIFY_REG(i2c->TRISE, I2C_TRISE_TRISE, (input_clock_mhz + 1)<<I2C_TRISE_TRISE_Pos);

	SET_BIT(i2c->CR1, I2C_CR1_PE);
}


bool i2c_write(I2C_TypeDef *i2c, uint8_t dev_addr, uint8_t *buf, uint16_t tx_size, uint32_t timeout)
{
#ifdef RTOS
	uint32_t current_tick = xTaskGetTickCount();
#elif
	uint32_t current_tick = get_tick();
#endif
	while (READ_BIT(i2c->SR2, I2C_SR2_BUSY))
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

	CLEAR_BIT(i2c->CR1, I2C_CR1_POS);
	SET_BIT(i2c->CR1, I2C_CR1_START);
	while (!READ_BIT(i2c->SR1, I2C_SR1_SB))
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

	send_dev_addr(i2c, dev_addr, WRITE);
	/* Wait till slave ACK */
	while (!READ_BIT(i2c->SR1, I2C_SR1_AF) && !READ_BIT(i2c->SR1, I2C_SR1_ADDR))
	{
#ifdef RTOS
				if ((xTaskGetTickCount() - current_tick) > timeout)
				{
					SET_BIT(i2c->CR1, I2C_CR1_STOP);
					CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
					return false;
				}
#elif
				if ((get_tick() - current_tick) > timeout)
				{
					SET_BIT(i2c->CR1, I2C_CR1_STOP);
					CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
					return false;
				}
#endif
	}
	/* Dummy reads to reset ADDR bit */
	i2c->SR1;
	i2c->SR2;
	/* Send data */
	for (uint16_t i = 0; i < tx_size; i++)
	{
		if (!send_data_byte(i2c, &buf[i]))
		{
			return false;
		}
	}
	SET_BIT(i2c->CR1, I2C_CR1_STOP);

	return true;
}

bool i2c_read(I2C_TypeDef *i2c, uint8_t dev_addr, uint8_t *buf, uint16_t rx_size, uint32_t timeout)
{
#ifdef RTOS
	uint32_t current_tick = xTaskGetTickCount();
#elif
	uint32_t current_tick = get_tick();
#endif

	while (READ_BIT(i2c->SR2, I2C_SR2_BUSY))
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

	CLEAR_BIT(i2c->CR1, I2C_CR1_POS);
	SET_BIT(i2c->CR1, I2C_CR1_START);
	while (!READ_BIT(i2c->SR1, I2C_SR1_SB))
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

	send_dev_addr(i2c, dev_addr, READ);
	/* Wait till slave ACK */
	while (!READ_BIT(i2c->SR1, I2C_SR1_AF) && !READ_BIT(i2c->SR1, I2C_SR1_ADDR))
	{
#ifdef RTOS
		if ((xTaskGetTickCount() - current_tick) > timeout)
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
#elif
		if ((get_tick() - current_tick) > timeout)
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
	#endif
	}
	/* Dummy reads to reset ADDR bit */
	i2c->SR1;
	i2c->SR2;

	/*Read data*/
	if (!read_data_bytes(i2c, buf, rx_size, current_tick, timeout))
	{
		return false;
	}

	SET_BIT(i2c->CR1, I2C_CR1_STOP);

	return true;

}

bool i2c_mem_write(I2C_TypeDef *i2c, uint8_t dev_addr, uint16_t reg_addr, Reg_mem_size_et e_reg_size, uint8_t *buf, uint16_t tx_size, uint32_t timeout)
{
#ifdef RTOS
	uint32_t current_tick = xTaskGetTickCount();
#elif
	uint32_t current_tick = get_tick();
#endif
	while (READ_BIT(i2c->SR2, I2C_SR2_BUSY))
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
	CLEAR_BIT(i2c->CR1, I2C_CR1_POS);
	SET_BIT(i2c->CR1, I2C_CR1_START);
	while (!READ_BIT(i2c->SR1, I2C_SR1_SB))
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

	if (!mem_write_data(i2c, dev_addr, reg_addr, e_reg_size, buf, tx_size, current_tick, timeout))
	{
		return false;
	}

	SET_BIT(i2c->CR1, I2C_CR1_STOP);

	return true;
}

bool i2c_mem_read(I2C_TypeDef *i2c, uint8_t dev_addr, uint16_t reg_addr, Reg_mem_size_et e_reg_size, uint8_t *buf, uint16_t rx_size, uint32_t timeout)
{
#ifdef RTOS
	uint32_t current_tick = xTaskGetTickCount();
#elif
	uint32_t current_tick = get_tick();
#endif
	while (READ_BIT(i2c->SR2, I2C_SR2_BUSY))
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

	CLEAR_BIT(i2c->CR1, I2C_CR1_POS);
	SET_BIT(i2c->CR1, I2C_CR1_START);
	while (!READ_BIT(i2c->SR1, I2C_SR1_SB))
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

	if (!mem_read_data(i2c, dev_addr, reg_addr, e_reg_size, buf, rx_size, current_tick, timeout))
	{
		return false;
	}

	SET_BIT(i2c->CR1, I2C_CR1_STOP);

	return true;
}

static bool mem_write_data(I2C_TypeDef *i2c, uint8_t dev_addr, uint16_t reg_addr, Reg_mem_size_et e_reg_size, uint8_t *buf, uint16_t tx_size, uint32_t current_tick, uint32_t timeout)
{
	send_dev_addr(i2c, dev_addr, WRITE);
	/* Wait till slave ACK */
	while (!READ_BIT(i2c->SR1, I2C_SR1_AF) && !READ_BIT(i2c->SR1, I2C_SR1_ADDR))
	{
#ifdef RTOS
		if ((xTaskGetTickCount() - current_tick) > timeout)
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
#elif
		if ((get_tick() - current_tick) > timeout)
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
#endif
	}
	/* Dummy reads to reset ADDR bit */
	i2c->SR1;
	i2c->SR2;
	/* Send reg address */
	if (e_reg_size == REG_8BIT)
	{
		uint8_t tx_byte = (uint8_t)reg_addr;
		if (!send_data_byte(i2c, &tx_byte))
		{
			return false;
		}
	}
	else if (e_reg_size == REG_16BIT)
	{
		for (uint8_t i = 0; i < sizeof(uint16_t); i++)
		{
			uint8_t tx_byte = (reg_addr >> (sizeof(uint16_t) - i - 1) * 8);
			if (!send_data_byte(i2c, &tx_byte))
			{
				return false;
			}
		}
	}
	/* Send data */
	for (uint16_t i = 0; i < tx_size; i++)
	{
		if (!send_data_byte(i2c, &buf[i]))
		{
			return false;
		}
	}

	return true;
}

static bool read_data_bytes(I2C_TypeDef *i2c, uint8_t *buf, uint16_t rx_size, uint32_t current_tick, uint32_t timeout)
{
	for (uint16_t i = 0; i < rx_size; i++)
	{
		if (i < rx_size - 1)
		{
			SET_BIT(i2c->CR1, I2C_CR1_ACK);
			while (!READ_BIT(i2c->SR1, I2C_SR1_RXNE))
			{
#ifdef RTOS
				if ((xTaskGetTickCount() - current_tick) > timeout)
				{
					SET_BIT(i2c->CR1, I2C_CR1_STOP);
					return false;
				}
#elif
				if ((get_tick() - current_tick) > timeout)
				{
					SET_BIT(i2c->CR1, I2C_CR1_STOP);
					return false;
				}
#endif
			}
			buf[i] = i2c->DR;
		}
		else
		{
			/* Clear ACK bit right after reading of second last rx byte */
			CLEAR_BIT(i2c->CR1, I2C_CR1_ACK);
			while (!READ_BIT(i2c->SR1, I2C_SR1_RXNE))
			{
#ifdef RTOS
				if ((xTaskGetTickCount() - current_tick) > timeout)
				{
					SET_BIT(i2c->CR1, I2C_CR1_STOP);
					return false;
				}
#elif
				if ((get_tick() - current_tick) > timeout)
				{
					SET_BIT(i2c->CR1, I2C_CR1_STOP);
					return false;
				}
#endif
			}
			buf[i] = i2c->DR;
		}
	}

	return true;
}

static bool mem_read_data(I2C_TypeDef *i2c, uint8_t dev_addr, uint16_t reg_addr, Reg_mem_size_et e_reg_size, uint8_t *buf, uint16_t rx_size, uint32_t current_tick, uint32_t timeout)
{
	send_dev_addr(i2c, dev_addr, WRITE);
	/* Wait till slave ACK */
	while (!READ_BIT(i2c->SR1, I2C_SR1_AF) && !READ_BIT(i2c->SR1, I2C_SR1_ADDR))
	{
#ifdef RTOS
		if ((xTaskGetTickCount() - current_tick) > timeout)
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
#elif
		if ((get_tick() - current_tick) > timeout)
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
#endif
	}
	/* Dummy reads to reset ADDR bit */
	i2c->SR1;
	i2c->SR2;
	/* Send reg address */
	/* Send reg address */
	if (e_reg_size == REG_8BIT)
	{
		uint8_t tx_byte = (uint8_t)reg_addr;
		if (!send_data_byte(i2c, &tx_byte))
		{
			return false;
		}
	}
	else if (e_reg_size == REG_16BIT)
	{
		for (uint8_t i = 0; i < sizeof(uint16_t); i++)
		{
			uint8_t tx_byte = (reg_addr >> (sizeof(uint16_t) - i - 1) * 8);
			if (!send_data_byte(i2c, &tx_byte))
			{
				return false;
			}
		}
	}
	/* Start again */
	SET_BIT(i2c->CR1, I2C_CR1_START);
	while (!READ_BIT(i2c->SR1, I2C_SR1_SB))
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

	send_dev_addr(i2c, dev_addr, READ);
	/* Wait till slave ACK */
	while (!READ_BIT(i2c->SR1, I2C_SR1_AF) && !READ_BIT(i2c->SR1, I2C_SR1_ADDR))
	{
#ifdef RTOS
		if ((xTaskGetTickCount() - current_tick) > timeout)
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
#elif
		if ((get_tick() - current_tick) > timeout)
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
#endif
	}
	/* Dummy reads to reset ADDR bit */
	i2c->SR1;
	i2c->SR2;

	/*Read data*/
	if (!read_data_bytes(i2c, buf, rx_size, current_tick, timeout))
	{
		return false;
	}

	return true;
}



static bool send_data_byte(I2C_TypeDef *i2c, uint8_t *buf)
{
	i2c->DR = *buf;
	while (!READ_BIT(i2c->SR1, I2C_SR1_TXE))
	{
		if (READ_BIT(i2c->SR1, I2C_SR1_AF))
		{
			SET_BIT(i2c->CR1, I2C_CR1_STOP);
			CLEAR_BIT(i2c->SR1, I2C_SR1_AF);
			return false;
		}
	}
	return true;
}

static void send_dev_addr(I2C_TypeDef *i2c, uint8_t dev_addr, Action_et action)
{
	if (action == WRITE)
	{
		i2c->DR = dev_addr << 1;
	}
	else if (action == READ)
	{
		i2c->DR = (dev_addr << 1) | 1;
	}
}

static void sw_reset(void)
{
	SET_BIT(I2C3->CR1, I2C_CR1_SWRST);
	while (!READ_BIT(I2C3->CR1, I2C_CR1_SWRST));
	CLEAR_BIT(I2C3->CR1, I2C_CR1_SWRST);
	while (READ_BIT(I2C3->CR1, I2C_CR1_SWRST));
}

void I2C3_EV_IRQHandler()
{

}


