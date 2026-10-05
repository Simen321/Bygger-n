#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void spi_init(void);
uint8_t spi_transfer(uint8_t data);
uint8_t spi_write_read(uint8_t data);

void spi_test(void);
void spi_select_slave(uint8_t nmb);



#endif