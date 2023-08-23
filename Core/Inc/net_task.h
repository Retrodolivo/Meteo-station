#ifndef INC_NET_TASK_H_
#define INC_NET_TASK_H_

#include <stdbool.h>

#include "main.h"

typedef struct
{
	uint8_t ip[4];
	uint16_t port;
	uint8_t name[15];		/*holds xxx.xxx.xxx.xxx pattern*/
	uint8_t wizchip_socket;
} Server_params_st;

bool net_init(void);
void net_task(void *params);
bool re_init_net(void);

#endif /* HTTPCLIENT_H_ */
