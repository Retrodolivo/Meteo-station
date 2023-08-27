#include <stdio.h>
#include <string.h>

#include "net_task.h"
#include "http_client.h"
#include "main_task.h"
#include "json.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "wizchip.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define TX_HTTP_BUF_SIZE		2048
#define RX_HTTP_BUF_SIZE		2048

static uint8_t tx_buf[TX_HTTP_BUF_SIZE];
static uint8_t rx_buf[RX_HTTP_BUF_SIZE];

extern QueueHandle_t meas_points_queue;


Server_params_st main_server;

static uint8_t http_answer_ok[] = "HTTP/1.1 200 OK";
static bool was_accepted = false;

static uint8_t main_sensor_endpoint[] = "/main-sensor";
static uint8_t keep_alive_connection[] = "keep-alive";
static uint8_t json_content_type[] = "application/json";

static uint8_t json_buf[512];

static void send_json(HttpRequest *request, uint8_t *json_buf);
static void ip2name(uint8_t *ip_buf, uint8_t *name_buf);
static void flush_tx_buf(void);
static void flush_rx_buf(void);

bool net_init(void)
{
	bool ret_status = false;
	bool http_ok = false;
	bool wizchip_ok = false;

	wizchip_ok = w5500_init(SPI2);

	main_server.ip[0] = 169;
	main_server.ip[1] = 254;
	main_server.ip[2] = 223;
	main_server.ip[3] = 76;
	main_server.port = 80;
	main_server.wizchip_socket = 0;
	ip2name(main_server.ip, main_server.name);

	/*connection is stable after reset only in 'keep-alive'*/
	/*POST points*/
	request.method = (uint8_t *)HTTP_POST;
	request.uri = main_sensor_endpoint;
	request.host = main_server.name;
	request.connection = keep_alive_connection;
	request.content_type = json_content_type;

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
			static Point_st temp[MAX_POINTS];
			if (xQueueReceive(meas_points_queue, &temp, 0))
			{
				httpc_connect();
				json_create(json_buf, temp, MAX_POINTS);
				send_json(&request, json_buf);
				flush_tx_buf();
			}
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

static void send_json(HttpRequest *request, uint8_t *json_buf)
{
	uint16_t content_len = strlen((char *)json_buf);
	uint32_t headers_len = httpc_form_req_no_body(request, tx_buf, content_len);
	for (uint16_t i = 0; i < content_len; i++)
	{
		tx_buf[headers_len + i] = json_buf[i];
	}
	http_send_buf(tx_buf, headers_len + content_len);
	flush_tx_buf();
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
