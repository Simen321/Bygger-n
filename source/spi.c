#include "../include/spi.h"

#include <avr/io.h>
#include <avr/delay.h>
#include <stdio.h>

uint8_t spi_write_read(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & (1 << SPIF))){  }
    return SPDR;

}

void spi_init(void) {
    // Sett den EKTE maskinvare-SS (PB4), samt MOSI (PB5) og SCK (PB7) som utganger
    DDRB |= (1 << PB1) | (1 << PB4) | (1 << PB5) | (1 << PB7);

    // Aktiver spi og konfigurer den til master
    SPCR = (1 << SPE) | (1 << MSTR);

    // Sett hastigheten til dobbel hastighet 
    SPSR |= (1 << SPI2X); // SPI Status Register

    PORTB |= (1 << PB1);
    PORTB |= (1 << PB4);
    DDRB &= ~(1 << PB6);
}

void spi_select_slave_n(uint8_t n) {
    PORTB |= (1 << PB1); // HIGH
    PORTB |= (1 << PB4); // HIGH
    PORTE |= (1 << PE2); // HIGH

    switch (n) {
        case 1:
            PORTB &= ~(1 << PB4); // CS-PIn IO
        case 2:
            PORTB &= ~(1 << PB1); // CS-pin OLED
        case 3:
            PORTE &= ~(1<<PE2); // CS-pin MCP2515
    }
}

// make these just because lab notes says so, even though read and write are the same
void spi_write_byte(uint8_t data) { 
    (void)spi_write_read(data);
}

void spi_write_n_bytes(const uint8_t *tx_buffer, size_t n) {
    for (size_t i = 0; i < n; i++) {
        spi_write_byte(tx_buffer[i]);
    }
}

uint8_t spi_read_byte(void) {
    return spi_write_read(0xFF);
}

void spi_read_n_bytes(uint8_t *rx_buffer, size_t n){
    for (size_t i = 0; i < n; i++) {
        rx_buffer[i] = spi_read_byte();
    }
}
