#include <string.h>

#include "main.h"
#include "delay.h"
#include "rcc.h"
#include "i2c.h"
#include "usart.h"
#include "bme280.h"
#include "dma.h"
#include "lcd_nextion.h"

void led_init(void);

bme280_t bme280;
status_bme280_t bme_status;

uint8_t buf[] = "Hello";

int main(void)
{
	rcc_config();
	delay_ms_init(SystemCoreClock);
	i2c_init(I2C3);
	usart_init(USART2, 9600);
	lcd_init(USART2);
	led_init();
	dma1_init();

	bme280.i2c_port = I2C3;
	bme280.i2c_addr = 0x76;

	bme_status = bme280_init(&bme280, OSRS_2, OSRS_2, OSRS_2, MODE_FORCED, T_SB_0p5, IIR_4);

	while (1)
	{
		bme280_measure(&bme280);
		float temp = bme280.temperature;
		float press = bme280.pressure;
		float humid = bme280.humidity;

		lcd_write(buf, strlen((char *)buf));

		GPIOD->ODR ^= GPIO_ODR_OD13;

		delay_ms(1000);
	}
}


void led_init(void)
{
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN);

	MODIFY_REG(GPIOD->MODER, GPIO_MODER_MODER13, 1<<GPIO_MODER_MODER13_Pos);
	CLEAR_BIT(GPIOD->OTYPER, GPIO_OTYPER_OT_13);
	MODIFY_REG(GPIOD->OSPEEDR, GPIO_OSPEEDR_OSPEED13, 0 << GPIO_OSPEEDR_OSPEED13_Pos);
	MODIFY_REG(GPIOD->PUPDR, GPIO_PUPDR_PUPD13, 0 << GPIO_PUPDR_PUPD13_Pos);

	CLEAR_BIT(GPIOD->ODR, GPIO_ODR_OD13);
}

