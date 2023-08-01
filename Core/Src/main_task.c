#include <string.h>
#include <stdio.h>

#include "main_task.h"
#include "lcd_nextion.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern TaskHandle_t main_task_handle;
extern TaskHandle_t main_sensor_task_handle;
extern TaskHandle_t net_task_handle;

extern QueueHandle_t init_state_queue;
extern QueueHandle_t sensor_data_queue;

Sensor_data_st main_sensor_meas;

static bool show_sensor_data(Sensor_data_st *meas, uint32_t timeout);
static void resume_tasks(void);

void main_task(void *params)
{
	Init_state_st init_state;

	xQueueReceive(init_state_queue, &init_state, portMAX_DELAY);

	lcd_set_page(START_PAGE, 100);
	if (init_state.sensor_ok)
	{
		lcd_set_txt_color(SENS_INIT_STATE_LBL, PASS_COLOR_DEC, 100);
		lcd_set_txt(SENS_INIT_STATE_LBL, "OK", 100);
	}
	else
	{
		lcd_set_txt_color(SENS_INIT_STATE_LBL, ERROR_COLOR_DEC, 100);
		lcd_set_txt(SENS_INIT_STATE_LBL, "FAIL", 100);
	}
	if (init_state.net_ok)
	{
		lcd_set_txt_color(NET_INIT_STATE_LBL, PASS_COLOR_DEC, 100);
		lcd_set_txt(NET_INIT_STATE_LBL, "OK", 100);
	}
	else
	{
		lcd_set_txt_color(NET_INIT_STATE_LBL, ERROR_COLOR_DEC, 100);
		lcd_set_txt(NET_INIT_STATE_LBL, "FAIL", 100);
	}

	if (init_state.sensor_ok && init_state.net_ok)
	{
		lcd_set_txt_color(TOTAL_INIT_STATE_LBL, PASS_COLOR_DEC, 100);
		lcd_set_txt(TOTAL_INIT_STATE_LBL, "SUCCESSED", 100);
		vTaskDelay(pdMS_TO_TICKS(1500));
		lcd_set_page(MAIN_PAGE, 100);
		resume_tasks();
	}
	else
	{
		lcd_set_txt_color(TOTAL_INIT_STATE_LBL, ERROR_COLOR_DEC, 100);
		lcd_set_txt(TOTAL_INIT_STATE_LBL, "FAILED", 100);
		vTaskSuspend(NULL);
	}

	while (1)
	{
		xQueueReceive(sensor_data_queue, &main_sensor_meas, portMAX_DELAY);
		show_sensor_data(&main_sensor_meas, 200);

	}
}

static bool show_sensor_data(Sensor_data_st *meas, uint32_t timeout)
{
	char temp[50];

	sprintf(temp, "%.2f C", meas->tempr);
	if (!lcd_set_txt(TEMPER_VAL_LBL, temp, timeout))
		return false;
	sprintf(temp, "%.0f Pa", meas->press);
	if (!lcd_set_txt(PRESS_VAL_LBL, temp, timeout))
		return false;
	sprintf(temp, "%.0f %%", meas->humid);
	if (!lcd_set_txt(HUMID_VAL_LBL, temp, timeout))
		return false;

	return true;
}

static void resume_tasks(void)
{
	vTaskResume(main_sensor_task_handle);
	vTaskResume(net_task_handle);
}

