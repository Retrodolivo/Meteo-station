#ifndef INC_MAIN_TASK_H_
#define INC_MAIN_TASK_H_

#include "main.h"

#define MAX_POINTS	3

typedef struct
{
	float tempr;
	float press;
	float humid;
} Sensor_data_st;

typedef struct
{
	Sensor_data_st sensor_data;
	uint32_t timestamp;
} Point_st;


void main_task(void *params);


#endif /* INC_MAIN_TASK_H_ */
