#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stddef.h>

void spi_init(void);
void spi_select_slave_n(uint8_t n);
void spi_write_byte(uint8_t data);
void spi_write_n_bytes(const uint8_t *tx_buffer, size_t n); // use pointer to the array
uint8_t spi_read_byte(void);
void spi_read_n_bytes(uint8_t *rx_buffer, size_t n); // same here


#endif