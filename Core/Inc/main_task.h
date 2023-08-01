#ifndef INC_MAIN_TASK_H_
#define INC_MAIN_TASK_H_

#include "main.h"


typedef struct
{
	float tempr;
	float press;
	float humid;
} Sensor_data_st;


void main_task(void *params);


#endif /* INC_MAIN_TASK_H_ */
