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
    DDRB |= (1 << PB1) | (1 << PB4) | (1 << PB5) | (1 << PB7);

    // Aktiver SPI, sett til Master
    SPCR = (1 << SPE) | (1 << MSTR);

    // Dobbel hastighet (F_CPU / 2)
    //SPSR |= (1 << SPI2X);

    PORTB |= (1 << PB1);
    PORTB |= (1 << PB4);
    DDRB &= ~(1 << PB6);
}

uint8_t selected = 0;



void spi_select_slave(uint8_t nmb) 
{
    // set SS signal low på den enheten vi vil ha. men da må vi skru av de andre
    //PB1 = OLED CS
    //PB4 = IO CS
    // SS1 
    PORTB |= (1 << PB1); // HIGH
    PORTB |= (1 << PB4); // HIGH
    if (nmb == 1) {
        PORTB &= ~(1 << PB4);
    }
    if (nmb == 2) { // OLED
        PORTB &= ~(1 << PB1);
    }
    
    /*if (nmb == 0) {
        PORTB |= (1 << PB1); // HIGH
        PORTB |= (1 << PB4); // HIGH
        printf("selected nothing\n\r");
        selected = nmb;
    } else if (nmb == 1) {
        PORTB &= ~(1 << PB1); // low
        PORTB |= (1 << PB4); // high
        printf("selected IO\n\r");
        selected = nmb;
    }else if (nmb == 2) {
        PORTB &= ~(1 << PB1); // LOW
        PORTB |= (1 << PB4); 
        printf("selected OLED\n\r");

        selected = nmb;
    }*/
}


uint8_t spi_write_read(uint8_t data)
{
    //spi_select_slave(address);

    SPDR = data;
    while (!(SPSR & (1 << SPIF)))
    {

    }
    //_delay_us(40);
    //spi_select_slave(address);
    return SPDR;

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