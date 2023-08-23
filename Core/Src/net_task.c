#include <stdio.h>
#include <string.h>

#include "net_task.h"
#include "http_client.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "wizchip.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define TX_HTTP_BUF_SIZE		2048
#define RX_HTTP_BUF_SIZE		2048

uint8_t tx_buf[TX_HTTP_BUF_SIZE];
uint8_t rx_buf[RX_HTTP_BUF_SIZE];

Server_params_st main_server;

uint8_t http_answer_ok[] = "HTTP/1.1 200 OK";
bool was_accepted = false;

static void ip2name(uint8_t *ip_buf, uint8_t *name_buf);
static void flush_tx_buf(void);
static void flush_rx_buf(void);

bool net_init(void)
{
	bool ret_status = false;
	bool http_ok = false;
	bool wizchip_ok = false;

	wizchip_ok = w5500_init(SPI2);

	main_server.ip[0] = 192;
	main_server.ip[1] = 168;
	main_server.ip[2] = 42;
	main_server.ip[3] = 89;
	main_server.port = 80;
	main_server.wizchip_socket = 0;
	ip2name(main_server.ip, main_server.name);

//	/*connection is stable after reset only in 'keep-alive'*/
//	/*POST power point*/
//	request_power_data.method = (uint8_t *)HTTP_POST;
//	request_power_data.uri = power_data_endpoint;
//	request_power_data.host = main_server.name;
//	request_power_data.connection = keep_alive_connection;
//	request_power_data.content_type = json_content_type;

	if (httpc_init(main_server.wizchip_socket, main_server.ip, main_server.port, tx_buf, rx_buf))
	{
		httpc_connection_handler();

		if (httpc_isSockOpen)
		{
			if (httpc_connect() == SOCK_OK)
			{
				http_ok = true;
			}
		}
	}

//	if (http_ok && wizchip_ok)
	if (wizchip_ok)
	{
		ret_status = true;
	}

	return ret_status;
}


void net_task(void *params)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		/*
		 * HTTP HANDLER
		 * */
		httpc_connection_handler();
		if (httpc_isSockOpen)
		{
			httpc_connect();
		}
		if(httpc_isConnected)
		{
//			if (xQueueReceive(tx_json_power_queue, &tx_json_power_buf, 0))
//			{
//				httpc_connect();
//				send_json(&request_power_data, tx_json_power_buf);
//			}
//
//			if (xQueueReceive(tx_json_status_queue, &tx_json_status_buf, 0))
//			{
//				httpc_connect();
//				send_json(&request_system_status, tx_json_status_buf);
//				NVIC_SystemReset();
//			}

			if(httpc_isReceived > 0)
			{
				uint16_t len = 0;

				len = httpc_recv(rx_buf, httpc_isReceived);
				if (strstr((char *)rx_buf, (char *)http_answer_ok) != NULL)
				{
					was_accepted = true;
				}
				else
				{
					was_accepted = false;
				}
				flush_rx_buf();
			}
		}
		xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(400));
	}
}


static void ip2name(uint8_t *ip_buf, uint8_t *name_buf)
{
	sprintf((char *)name_buf, "%d.%d.%d.%d", ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
}

static void flush_tx_buf(void)
{
	memset(tx_buf, 0, NUM_ELEMS(tx_buf));
}

static void flush_rx_buf(void)
{
	memset(rx_buf, 0, NUM_ELEMS(rx_buf));
}
