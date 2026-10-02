#include "../include/spi.h"

#include <avr/io.h>

/*void spi_init(void)
{
    // set the: PB5,4,7 (husker ikke hva vi satte opp i labben)
    // -MOSI
    // -SCK
    // -SS
    // as outputs
    DDRB |= (1 << PB5);
    DDRB |= (1 << PB7);
    DDRB |= (1 << PB4);

    // enable spi in master mode and clock rate?
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}*/
void spi_init(void)
{
    /*
     * MOSI
     * SCK
     * SS
     */
    DDRB |= (1 << PB5);
    DDRB |= (1 << PB7);
    DDRB |= (1 << PB4);


    /*
     * SPI enabled
     * Master
     *
     * SPR1 = 0
     * SPR0 = 0
     *
     * base = F_CPU / 4
     */
    SPCR =
        (1 << SPE) |
        (1 << MSTR);


    /*
     * Double SPI speed
     *
     * F_CPU / 2
     */
    SPSR |= (1 << SPI2X);
}

uint8_t spi_transfer(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & (1 << SPIF)))
    {

    }

    return SPDR;
}