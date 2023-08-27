#ifndef INC_JSON_H_
#define INC_JSON_H_

#include "main.h"
#include "main_task.h"

#define JBODY_START	'{'
#define JBODY_END	'}'

uint16_t json_create(uint8_t *buf, Point_st *point, uint16_t points_num);

#endif /* INC_JSON_H_ */
