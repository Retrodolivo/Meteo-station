#ifndef INC_BME280_H_
#define INC_BME280_H_

#include "main.h"
#include "i2c.h"

#define BME280_SCL_GPIO_PORT	GPIOA
#define BME280_SDA_GPIO_PORT	GPIOC

// Oversampling definitions
#define OSRS_OFF    	0x00
#define OSRS_1      	0x01
#define OSRS_2      	0x02
#define OSRS_4      	0x03
#define OSRS_8      	0x04
#define OSRS_16     	0x05

// MODE Definitions
#define MODE_SLEEP      0x00
#define MODE_FORCED     0x01
#define MODE_NORMAL     0x03

// Standby Time
#define T_SB_0p5    	0x00
#define T_SB_62p5   	0x01
#define T_SB_125    	0x02
#define T_SB_250    	0x03
#define T_SB_500    	0x04
#define T_SB_1000   	0x05
#define T_SB_10     	0x06
#define T_SB_20     	0x07

// IIR Filter Coefficients
#define IIR_OFF     	0x00
#define IIR_2       	0x01
#define IIR_4       	0x02
#define IIR_8       	0x03
#define IIR_16      	0x04


// REGISTERS DEFINITIONS
#define ID_REG      	0xD0
#define RESET_REG  		0xE0
#define CTRL_HUM_REG    0xF2
#define STATUS_REG      0xF3
#define CTRL_MEAS_REG   0xF4
#define CONFIG_REG      0xF5
#define PRESS_MSB_REG   0xF7

typedef enum
{
	OK = 0,
	WRITE_READ_ERR = -1,
	ID_FAIL = -2

} status_bme280_t;

typedef struct
{
	uint16_t dig_T1,
	         dig_P1,
	         dig_H1, dig_H3;

	int16_t  dig_T2, dig_T3,
	         dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9,
			 dig_H2,  dig_H4, dig_H5, dig_H6;
} comp_pars_t;

typedef struct
{
	/*manually add serial port params*/
	I2C_TypeDef *i2c_port;
	uint16_t i2c_addr;

	uint8_t chip_id;
	uint8_t trimdata[32];
	comp_pars_t comp_pars;
	uint32_t t_raw,
			 p_raw,
			 h_raw;
	float temperature,
		  pressure,
		  humidity;

	status_bme280_t status_dev;
} bme280_t;


status_bme280_t bme280_init(bme280_t *dev, uint8_t osrs_t, uint8_t osrs_p, uint8_t osrs_h, uint8_t mode, uint8_t t_sb, uint8_t filter);
void bme280_measure(bme280_t *dev);

#endif /* INC_BME280_H_ */
