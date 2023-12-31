#include <string.h>

#include "wizchip.h"
#include "spi.h"

#ifdef RTOS

	#include "FreeRTOS.h"
	#include "task.h"
#elif
	#include "delay.h"

#endif

static bool are_equal(uint8_t *arr1, uint8_t *arr2, uint8_t arrs_size);


w5500chip_t w5500;


static void w5500_select(void)
{
	CLEAR_BIT(W5500_CS_GPIO_PORT->ODR, GPIO_ODR_OD12);
}

static void w5500_unselect(void)
{
	SET_BIT(W5500_CS_GPIO_PORT->ODR, GPIO_ODR_OD12);
}

static uint8_t w5500_read_byte(void)
{
    uint8_t rb;

    spi_receive(w5500.spi_port, &rb, 1, 1000);

    return rb;
}

static void w5500_write_byte(uint8_t wb)
{
	spi_transmit(w5500.spi_port, &wb, 1, 1000);
}

static void w5500_hw_reset()
{
	CLEAR_BIT(W5500_RST_GPIO_PORT->ODR, GPIO_ODR_OD10);
#ifdef RTOS
	vTaskDelay(pdMS_TO_TICKS(100));
#elif
	delay_ms(100);
#endif
//	vTaskDelay(pdMS_TO_TICKS(100));
	SET_BIT(W5500_RST_GPIO_PORT->ODR, GPIO_ODR_OD10);
#ifdef RTOS
	vTaskDelay(pdMS_TO_TICKS(100));
#elif
	delay_ms(100);
#endif
//	vTaskDelay(pdMS_TO_TICKS(100));
}

bool w5500_init(SPI_TypeDef *spi)
{
	/* CS */
	MODIFY_REG(W5500_CS_GPIO_PORT->MODER, GPIO_MODER_MODER12, 0b01 << GPIO_MODER_MODER12_Pos);
	CLEAR_BIT(W5500_CS_GPIO_PORT->OTYPER, GPIO_OTYPER_OT12);
	MODIFY_REG(W5500_CS_GPIO_PORT->OSPEEDR, GPIO_OSPEEDR_OSPEED12, 0b11 << GPIO_OSPEEDR_OSPEED12_Pos);
	MODIFY_REG(W5500_CS_GPIO_PORT->PUPDR, GPIO_PUPDR_PUPD12, 0b01 << GPIO_PUPDR_PUPD12_Pos);
	SET_BIT(W5500_CS_GPIO_PORT->ODR, GPIO_ODR_OD12);

	bool status = true;

	w5500.spi_port = spi;
	/*link up architecture based spi cntrl funcs to lib funcs*/
	reg_wizchip_cs_cbfunc(w5500_select, w5500_unselect);
	reg_wizchip_spi_cbfunc(w5500_read_byte, w5500_write_byte);
	w5500_hw_reset();
	/*split up rxtx buffer in Kb among sockets*/
	w5500.rxtx_buff[SOCK_0] = 2;
	w5500.rxtx_buff[SOCK_1] = 2;
	w5500.rxtx_buff[SOCK_2] = 2;
	w5500.rxtx_buff[SOCK_3] = 2;
	w5500.rxtx_buff[SOCK_4] = 2;
	w5500.rxtx_buff[SOCK_5] = 2;
	w5500.rxtx_buff[SOCK_6] = 2;
	w5500.rxtx_buff[SOCK_7] = 2;

	uint8_t mac[6] = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef};
	memcpy(w5500.netinfo.mac, mac, 6);

	uint8_t ip[4] = {169, 254, 223, 80};
	memcpy(w5500.netinfo.ip, ip, 4);

	/*setting subnet mask*/
	uint8_t sn[4] = {255, 255, 0, 0};
	memcpy(w5500.netinfo.sn, sn, 4);

	/*setting gateway*/
	uint8_t gw[4] = {192, 168, 0, 1};
	memcpy(w5500.netinfo.gw, gw, 4);

	wizchip_init(w5500.rxtx_buff, w5500.rxtx_buff);

	wizchip_setnetinfo(&w5500.netinfo);
	wizchip_getnetinfo(&w5500.netinfo);
	/*after wizchip_getnetinfo() netinfo struct should stay the same. Checking for equality*/

	status = are_equal(w5500.netinfo.ip, ip, NUM_ELEMS(ip));
	status = are_equal(w5500.netinfo.mac, mac, NUM_ELEMS(mac));
	status = are_equal(w5500.netinfo.sn, sn, NUM_ELEMS(sn));
	status = are_equal(w5500.netinfo.gw, gw, NUM_ELEMS(gw));

	return status;
}

static bool are_equal(uint8_t *arr1, uint8_t *arr2, uint8_t arrs_size)
{
	bool ret = true;

	for (int i = 0; i < arrs_size; i++)
	{
		if (arr1[i] != arr2[i])
			ret = false;
	}

	return ret;
}
