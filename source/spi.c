#include "../include/spi.h"

#include <avr/io.h>
#include <avr/delay.h>
#include <stdio.h>

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
    // Sett den EKTE maskinvare-SS (PB4), samt MOSI (PB5) og SCK (PB7) som utganger
    DDRB |= (1 << PB4) | (1 << PB5) | (1 << PB7);

    // Aktiver SPI, sett til Master
    SPCR = (1 << SPE) | (1 << MSTR);

    // Dobbel hastighet (F_CPU / 2)
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

void spi_test() 
{
    uint8_t nmb = 100;

    uint8_t rslt = spi_transfer(nmb);
    printf("printed out %d\n\r", rslt);

}