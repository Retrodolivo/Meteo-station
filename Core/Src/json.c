#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "json.h"
#include "main_task.h"


static uint16_t json_add_pair_int_val(uint8_t *buf, char *key_name, uint32_t *vals, uint16_t vals_num, uint16_t enter_len, bool is_last);

uint16_t json_create(uint8_t *buf, Point_st *point, uint16_t points_num)
{
	uint32_t temper_temp[points_num];
	uint32_t press_temp[points_num];
	uint32_t humid_temp[points_num];
	uint32_t timestamp_temp[points_num];

	uint16_t len = 0;

	buf[len++] = JBODY_START;

	for (uint32_t i = 0; i < points_num; i++)
	{
		temper_temp[i] = point[i].sensor_data.tempr * 1000;
		press_temp[i] = point[i].sensor_data.press;
		humid_temp[i] = point[i].sensor_data.humid * 1000;
		timestamp_temp[i] = point[i].timestamp;
	}

	len = json_add_pair_int_val(buf, "T", temper_temp, points_num, len, false);
	len = json_add_pair_int_val(buf, "P", press_temp, points_num, len, false);
	len = json_add_pair_int_val(buf, "H", humid_temp, points_num, len, false);
	len = json_add_pair_int_val(buf, "time", timestamp_temp, points_num, len, true);

	buf[len] = JBODY_END;

	return len;
}

static uint16_t json_add_pair_int_val(uint8_t *buf, char *key_name, uint32_t *vals, uint16_t vals_num, uint16_t enter_len, bool is_last)
{
	uint16_t exit_len = enter_len;

	exit_len += sprintf((char *)buf + exit_len, "\"%s\":[", key_name);

	for (uint16_t i = 0; i < vals_num; i++)
	{
		//if last value
		if (i == vals_num - 1)
		{
			exit_len += sprintf((char *)buf + exit_len, "%d]", vals[i]);
		}
		else
		{
			exit_len += sprintf((char *)buf + exit_len, "%d,", vals[i]);
		}
	}
	if (!is_last)
	{
		exit_len += sprintf((char *)buf + exit_len, ",");
	}

	return exit_len;
}


