/**
 * Meteo-station app
 * @author RetroBoyy
 * Description: FreeRTOS based STM32 app
 */

#include <string.h>

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#include "rcc.h"
#include "i2c.h"
#include "usart.h"
#include "spi.h"
#include "dma.h"
#include "bme280.h"
#include "wizchip.h"
#include "lcd_nextion.h"

#define DWT_CTRL	(*(volatile uint32_t *)0xE0001000)

TaskHandle_t init_task_handle;
TaskHandle_t main_task_handle;
TaskHandle_t lcd_task_handle;
TaskHandle_t main_sensor_task_handle;
TaskHandle_t net_task_handle;

static void init_task(void *params);
static void lcd_task(void *params);
static void main_sensor_task(void *params);
static void net_task(void *params);

static void rtos_entities_init(void);
static void system_init(void);
void led_init(void);


bme280_t bme280;
status_bme280_t bme_status;
Init_state_st init_state;

uint8_t lcd_buf[256];
uint8_t lcd_msg[] = "Hello";

int main(void)
{
	system_init();
	NVIC_SetPriorityGrouping(0);
	DWT_CTRL |= (1<<0);
	SEGGER_SYSVIEW_Conf();
	SEGGER_SYSVIEW_Start();

	rtos_entities_init();
	vTaskStartScheduler();

	while (1)
	{

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

static void system_init(void)
{
	rcc_config();
	i2c_init(I2C3);
	spi_init(SPI2);
	usart_init(USART2, 9600);
	dma1_init();
}

static void rtos_entities_init(void)
{
	BaseType_t status;

	status = xTaskCreate(init_task, "init_task", configMINIMAL_STACK_SIZE, NULL, 3, &init_task_handle);
	configASSERT(status == pdPASS);

}

static void init_task(void *params)
{
	BaseType_t status;

	init_state.sensor_ok = false;
	init_state.net_ok = false;

	bme280.i2c_port = I2C3;
	bme280.i2c_addr = 0x76;

	while (1)
	{
		status = xTaskCreate(main_task, "main_task", configMINIMAL_STACK_SIZE * 2, NULL, 2, &main_task_handle);
		configASSERT(status == pdPASS);

		lcd_init(USART2, lcd_buf);
		status = xTaskCreate(lcd_task, "lcd_task", configMINIMAL_STACK_SIZE, NULL, 2, &lcd_task_handle);
		configASSERT(status == pdPASS);

		if (bme280_init(&bme280, OSRS_2, OSRS_2, OSRS_2, MODE_FORCED, T_SB_0p5, IIR_4) == OK)
		{
			init_state.sensor_ok = true;
			status = xTaskCreate(main_sensor_task, "main_sensor_task", configMINIMAL_STACK_SIZE * 2, NULL, 2, &main_sensor_task_handle);
			configASSERT(status == pdPASS);
		}
		if (w5500_init(SPI2))
		{
			init_state.net_ok = true;
			status = xTaskCreate(net_task, "net_task", configMINIMAL_STACK_SIZE * 2, NULL, 2, &net_task_handle);
			configASSERT(status == pdPASS);
		}

		if (init_state.sensor_ok && init_state.net_ok)
		{
			/* write on 'OK' page  */
			//lcd_write(buf, strlen((char *)buf));
		}
		else
		{
			/* Write all statuses on lcd */

		}
		vTaskSuspend(NULL);
	}
}

static void lcd_task(void *params)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		lcd_write(lcd_msg, strlen((char *)lcd_msg), 100);
		lcd_write(lcd_msg, strlen((char *)lcd_msg), 100);

		xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
	}
}

static void main_sensor_task(void *params)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		bme280_measure(&bme280);
		float temp = bme280.temperature;
		float press = bme280.pressure;
		float humid = bme280.humidity;

		xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
	}
}

static void net_task(void *params)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
	}
}

