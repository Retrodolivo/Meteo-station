#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "stm32f4xx.h"

#define RTOS
#define NUM_ELEMS(x)	(sizeof(x) / sizeof(x[0]))

typedef struct
{
	bool sensor_ok;
	bool net_ok;
} Init_state_st;

#ifndef RTOS
#include "delay.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
