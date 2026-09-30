#define F_CPU 4915200UL
#define BAUD 9600UL
#define UBRR_VAL ((F_CPU) / (16 * BAUD) - 1) // teh baudrate formula

#include "../include/adc.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>



void adc_clock_init(void) 
{
    DDRB |= (1 << PB0);

    // ctc mode no presclaer
    TCCR0 = (1 << WGM01) |
            (1 << COM00) |
            (1 << CS00);
    OCR0 = 0;
}

uint8_t adc_read(uint8_t channel)
{
    *ADC_ADDR = 0x00;

    _delay_us(30);

    uint8_t data = 0;
    
    for (uint8_t i = 0; i <= channel; i++) {
        data = *ADC_ADDR;
    }
    
    return data;
}