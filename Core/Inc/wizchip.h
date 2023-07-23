#ifndef INC_WIZCHIP_H_
#define INC_WIZCHIP_H_

#include <stdbool.h>

#include "main.h"
#include "socket.h"

#define W5500_MISO_GPIO_PORT	GPIOB
#define W5500_MOSI_GPIO_PORT	GPIOB
#define W5500_CLK_GPIO_PORT		GPIOB
#define W5500_CS_GPIO_PORT		GPIOB

#define W5500_RST_GPIO_PORT		GPIOD



typedef enum
{
	SOCK_0,
	SOCK_1,
	SOCK_2,
	SOCK_3,
	SOCK_4,
	SOCK_5,
	SOCK_6,
	SOCK_7
}SOCK_t;

typedef struct
{
	SPI_TypeDef *spi_port;
	wiz_NetInfo netinfo;
	uint8_t rxtx_buff[8];
} w5500chip_t;


bool w5500_init(SPI_TypeDef *spi);


#endif /* INC_WIZCHIP_H_ */
