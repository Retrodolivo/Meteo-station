#ifndef INC_LCD_NEXTION_H_
#define INC_LCD_NEXTION_H_

#include <stdbool.h>

#include "main.h"
#include "usart.h"

/* LCD OBJECTS */
/* Pages */
#define START_PAGE 	"startup_page"
#define MAIN_PAGE 	"main_page"
#define INFO_PAGE 	"info_page"
/* Buttons */
#define INFO_BTN	"b_info"
#define BACK_BTN	"b_back"
/* Txt Labels */
#define SENS_INIT_STATE_LBL		"t_sensor_init"
#define NET_INIT_STATE_LBL		"t_net_init"
#define TOTAL_INIT_STATE_LBL	"t_total_state"

#define TEMPER_VAL_LBL			"t_T"
#define PRESS_VAL_LBL			"t_P"
#define HUMID_VAL_LBL			"t_H"

#define TIME_VAL_LBL			"t_time"
#define DATA_VAL_LBL			"t_date"

#define IP_VAL_LBL				"t_ip"
/* Misc */
#define ERROR_COLOR_DEC			"61440"
#define PASS_COLOR_DEC			"19722"
#define TERM_SYMB				0xFF
#define TERM_SYMB_AMOUNT		3


typedef struct
{
	USART_TypeDef *uart;
	uint8_t *txbuf;
	uint8_t msg_terminator[TERM_SYMB_AMOUNT];
	uint16_t last_msg_len;
} LCD_st;

void lcd_init(USART_TypeDef *uart, uint8_t *txbuf);
bool lcd_write(uint8_t *string, uint16_t len, uint32_t timeout);
bool lcd_set_page(char *page, uint32_t timeout);
bool lcd_set_txt_color(char *label, char *color, uint32_t timeout);
bool lcd_set_txt(char *label, char *txt, uint32_t timeout);


#endif /* INC_LCD_NEXTION_H_ */
