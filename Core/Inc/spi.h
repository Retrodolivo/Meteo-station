#ifndef INC_SPI_H_
#define INC_SPI_H_

#include <stdbool.h>
#include "main.h"

/**
 * Serial Peripheral Interface initializing
 * @param spi Pointer to spi structure which is going to be initialized
 */
void spi_init(SPI_TypeDef *spi);

/**
 * spi_transmit() send data through spi
 * non-blocking function. Unblocks after timeout
 * @param spi Pointer to spi structure port which is going to be used
 * @param txbuf uint8_t array that contains data that would be sent by spi
 * @param txlen txlen uint16_t var that contains amount of elems in @txbuf
 * @param timeout uint32_t var that specifies time in milliseconds which func waiting for the flags
 * @return return true if transfer had completed before timeout struck
 */
bool spi_transmit(SPI_TypeDef *spi, uint8_t *txbuf, uint16_t txlen, uint32_t timeout);
/**
 * spi_receive() receive data via spi
 * blocking function(no timeout). RTOS is responsible for unblock
 * @param spi Pointer to spi structure port which is going to be used
 * @param rxbuf uint8_t array that contains received data by spi
 * @param rxlen uint16_t var that contains amount received data
 * @return return true if receiving of @rxlen number of data had completed before timeout struck
 */
bool spi_receive(SPI_TypeDef *spi, uint8_t *rxbuf, uint16_t rxlen, uint32_t timeout);

#endif /* INC_SPI_H_ */
