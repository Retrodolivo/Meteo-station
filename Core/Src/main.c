/**
 * Meteo-station app
 * @author RetroBoyy
 * Description: FreeRTOS based STM32 app
 */

#include <string.h>

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "rcc.h"
#include "rtc.h"
#include "i2c.h"
#include "usart.h"
#include "spi.h"
#include "dma.h"
#include "bme280.h"
#include "wizchip.h"
#include "lcd_nextion.h"
#include "main_task.h"
#include "net_task.h"

#define DWT_CTRL	(*(volatile uint32_t *)0xE0001000)

TaskHandle_t init_task_handle;
TaskHandle_t main_task_handle;
TaskHandle_t main_sensor_task_handle;
TaskHandle_t net_task_handle;

TimerHandle_t datetime_timer;

QueueHandle_t init_state_queue;
QueueHandle_t sensor_data_queue;
QueueHandle_t datetime_queue;

static void init_task(void *params);
static void main_sensor_task(void *params);
extern void net_task(void *params);
extern void datetime_timer_callback(TimerHandle_t timer_handle);
extern void main_task(void *params);

static void rtos_entities_init(void);
static void system_init(void);
void led_init(void);


bme280_t main_sensor;
Init_state_st init_state;
extern Sensor_data_st main_sensor_meas;
uint8_t lcd_buf[256];
DateTime_st datetime;

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
	MODIFY_REG(GPIOD->OSPEEDR, GPIO_OSPEEDR_OSPEED13, 0<<GPIO_OSPEEDR_OSPEED13_Pos);
	MODIFY_REG(GPIOD->PUPDR, GPIO_PUPDR_PUPD13, 0<<GPIO_PUPDR_PUPD13_Pos);

	CLEAR_BIT(GPIOD->ODR, GPIO_ODR_OD13);
}

static void system_init(void)
{
	rcc_config();
	i2c_init(I2C3);
	spi_init(SPI2);
	usart_init(USART2, 9600);
	dma1_init();

	if (!READ_BIT(RTC->ISR, RTC_ISR_INITS))
	{
		rtc_init();

		datetime.year = 23;
		datetime.month = 8;
		datetime.date = 20;
		datetime.hours = 12;
		datetime.minutes = 0;
		datetime.seconds = 0;

		rtc_set_datetime(&datetime);
	}

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

	main_sensor.i2c_port = I2C3;
	main_sensor.i2c_addr = 0x76;

	while (1)
	{
		lcd_init(USART2, lcd_buf);

		if (bme280_init(&main_sensor, OSRS_2, OSRS_2, OSRS_2, MODE_FORCED, T_SB_0p5, IIR_4) == OK)
		{
			init_state.sensor_ok = true;
			status = xTaskCreate(main_sensor_task, "main_sensor_task", configMINIMAL_STACK_SIZE * 2, NULL, 2, &main_sensor_task_handle);
			configASSERT(status == pdPASS);
			vTaskSuspend(main_sensor_task_handle);
			sensor_data_queue = xQueueCreate(5, sizeof(Sensor_data_st));
		}
		if (net_init())
		{
			init_state.net_ok = true;
			status = xTaskCreate(net_task, "net_task", configMINIMAL_STACK_SIZE * 2, NULL, 2, &net_task_handle);
			configASSERT(status == pdPASS);
			vTaskSuspend(net_task_handle);
		}

		status = xTaskCreate(main_task, "main_task", configMINIMAL_STACK_SIZE * 3, NULL, 2, &main_task_handle);
		configASSERT(status == pdPASS);

		init_state_queue = xQueueCreate(1, sizeof(Init_state_st));
		configASSERT(init_state_queue != NULL);
		xQueueOverwrite(init_state_queue, &init_state);

		datetime_timer = xTimerCreate("datetime_timer", pdMS_TO_TICKS(900), pdTRUE, NULL, datetime_timer_callback);
		configASSERT(datetime_timer != NULL);
		xTimerStart(datetime_timer, 0);

		datetime_queue = xQueueCreate(1, sizeof(DateTime_st));
		configASSERT(datetime_queue != NULL);

		vTaskSuspend(NULL);
	}
}


static void main_sensor_task(void *params)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		if (bme280_measure(&main_sensor))
		{
			main_sensor_meas.tempr = main_sensor.temperature;
			main_sensor_meas.press = main_sensor.pressure;
			main_sensor_meas.humid = main_sensor.humidity;
			if (xQueueSend(sensor_data_queue, &main_sensor_meas, 100) != pdTRUE)
			{
				/*Error. Queue is full */
			}
		}
		else
		{
			/* Report about sensor no response */
		}

		xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
	}
}

