#ifndef INC_SPI_H_
#define INC_SPI_H_

#include <stdbool.h>
#include "main.h"

void spi_init(SPI_TypeDef *spi);
bool spi_transmit(SPI_TypeDef *spi, uint8_t *txbuf, uint16_t txlen, uint32_t timeout);
bool spi_receive(SPI_TypeDef *spi, uint8_t *rxbuf, uint16_t rxlen, uint32_t timeout);

#endif /* INC_SPI_H_ */
