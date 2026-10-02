#define F_CPU 4915200UL
#define BAUD 9600UL
#define UBRR_VAL ((F_CPU) / (16 * BAUD) - 1) // teh baudrate formula

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "labtools/sram_test_cr.c"
#include "include/adc.h"
#include "include/joystick.h"


int checkEdge(int currentEdge) { // Returns true if Rising Edge, returns false if Falling Edge
    static int previousEdge = 0; // hope this will continue to hold the signal as is.
    int output = 0;
    // Rising edge:
    if(!(previousEdge) && currentEdge) {
        output = 1;
    }

    // Falling edge:
    if ((previousEdge) && !(currentEdge)) {
        output = 2;
    }
    // keep the currentState by setting previousState to it now
    previousEdge = currentEdge;

    return output;
}

void initializeLedShow() {
    DDRA = 0xFF; // or DDRA = 0b11111111, set all the ports on A to output
}
void latchInit(){
    DDRB |= (1 << PB1);
}
void latchOn() { // trasparent
    PORTB |= (1 << PB1);
}
void latchOff() { // hold 
    PORTB &= ~(1 << PB1);
}

int sram_address_min = 0x000;
int sram_address_max = 0xBFF;
int dac_address_min = 0xC00;
int dac_address_max = 0xFFF;

void enable_external_sram(void) { // stolen directly from google after irritation <3
    // Set the SRE bit to enable the external memory interface
    //MCUCR |= (1 << SRE);
    // clear XMM2:XMM0 
    SFIOR &=    ~((1 << XMM2) | (1 << XMM1) | (1 << XMM0));
    
    SFIOR |= (1 << XMM2);
    SFIOR &= ~(1 << XMBK);

    EMCUCR &= ~((1 << SRL2) | (1 << SRL1) | (1 << SRL0));

    EMCUCR |= (1 << SRW11);
    MCUCR &= ~(1 << SRW10);
    MCUCR |= (1 << SRE);
}


volatile uint8_t dummy;
volatile uint8_t *sram = (volatile uint8_t *)0x1800;
volatile uint8_t *adc = (volatile uint8_t *) 0x1C00;





int main() {
    uart1_init();
    unsigned char recieved_text[10];
    int count = 0;
    fdevopen(uart_sendltr, uart_getltr);
    
    enable_external_sram();
    SRAM_test();

    adc_clock_init();

    joystick_calibration();
    while (1) {
        // adc_print_all();
        // JOYSTICK
        int16_t x = joystick_to_percent(joystickX_read());
        int16_t y = joystick_to_percent(joystickY_read());
        printf("Joystick positions, X: %d Y: %d :)\r\n", x, y);

        _delay_ms(50);
    }

    return 1;
}