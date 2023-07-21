#ifndef INC_I2C_H_
#define INC_I2C_H_

#include "main.h"

#include <stdbool.h>

typedef enum
{
	REG_8BIT,
	REG_16BIT
} Reg_mem_size_et;

typedef enum
{
	WRITE,
	READ
} Action_et;

void i2c_init(I2C_TypeDef *i2c);
bool i2c_write(I2C_TypeDef *i2c, uint8_t dev_addr, uint8_t *buf, uint16_t tx_size, uint32_t timeout);
bool i2c_read(I2C_TypeDef *i2c, uint8_t dev_addr, uint8_t *buf, uint16_t rx_size, uint32_t timeout);
bool i2c_mem_write(I2C_TypeDef *i2c, uint8_t dev_addr, uint16_t reg_addr, Reg_mem_size_et e_reg_size, uint8_t *buf, uint16_t tx_size, uint32_t timeout);
bool i2c_mem_read(I2C_TypeDef *i2c, uint8_t dev_addr, uint16_t reg_addr, Reg_mem_size_et e_reg_size, uint8_t *buf, uint16_t rx_size, uint32_t timeout);

#endif /* INC_I2C_H_ */
