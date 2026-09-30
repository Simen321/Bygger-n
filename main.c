#define F_CPU 4915200UL
#define BAUD 9600UL
#define UBRR_VAL ((F_CPU) / (16 * BAUD) - 1) // teh baudrate formula

//#define SRAM_ADDR ((volatile uint8_t *)0x1800)




#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>


#include "include/adc.h"
#include "include/joystick.h"

void uart1_init(void){
    UBRR1H = (uint8_t)(UBRR_VAL >> 8);
    UBRR1L = (uint8_t)(UBRR_VAL);
    
    UCSR1C =    (1 << URSEL1) | 
                (1 << UCSZ11) |
                (1 << UCSZ10);

    UCSR1B = (1 << TXEN1) | (1 << RXEN1); // Enabel transmitter and reciever
}

//UDRE // flag turns 1 when the buffer is ready for new data
// when UDRE is 0 process it
int uart_sendltr(char c, FILE *stream) {

    while (!(UCSR1A & (1 << UDRE1))); 
    // UCSR1A: Control and Status register      UDRE1: Data register
    // UDRE1:   https://onlinedocs.microchip.com/oxy/GUID-EC8D3BAB-0B5E-454F-AB6E-6A7C91C6F103-en-US-3/GUID-ECC601AE-780D-4643-BE2B-CB7717DB713E.html
    // UCSR1A:  https://onlinedocs.microchip.com/oxy/GUID-85E9B23E-BE9C-47EA-AED9-E8719EA2F7AF-en-US-2/GUID-73CB1E14-0C43-4A15-AA94-3D8AF6E17C6A.html
    UDR1 = c;

    return 0;
}

int uart_getltr(FILE *stream) {
    while (!(UCSR1A & (1 << RXC1))); 
    // UCSR1A: Control and Status register      RXC1: Is high when something is in the recieve dataregister, goes zero when the register is read.
    return UDR1;
    //unsigned char recieveed_data = UDR1; // UDR1 er da dataen bare, samme som i transmitt. Derfor er det avhengig av timingen.
    //return recieveed_data;
}
/*
void uart_sendltr(char c) {
    while (!(UCSR1A & (1 << UDRE1)));
    UDR1 = c;
}

unsigned char uart_getltr(void) {
    while (!(UCSR1A & (1 << RXC1)));
    unsigned char recieveed_data = UDR1; // UDR1 er da dataen bare, samme som i transmitt. Derfor er det avhengig av timingen.
    return recieveed_data;
}
*/


void cleanArray(unsigned char arr[10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = 0;
    }
}

//FILE *fdevopen(int (*uart_sendltr)(char, FILE *), int (*uart_getltr)(FILE *))


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
void theLedShow(int timeOffset) {
    PORTA = 0xFF;


    //latchOn();
    _delay_ms(250);
    latchOff();

    PORTA = 0b10101010;

    //latchOn();
    _delay_ms(250);
    //latchOff();

    PORTA = 0b01010101;

    //latchOn();
    _delay_ms(250);
    //latchOff();

    PORTA = 0x00;
    //latchOn();
    _delay_ms(250);
    //latchOff();
}

int sram_address_min = 0x000;
int sram_address_max = 0xBFF;
int dac_address_min = 0xC00;
int dac_address_max = 0xFFF;


void SRAM_test(void)
    {
		volatile char *ext_ram = (char *) 0x1000; // Start address for the SRAM
        uint16_t ext_ram_size = 0xC00;
        uint16_t write_errors = 0;
        uint16_t retrieval_errors = 0;
        printf("Starting SRAM test...\r\n");
        // rand() stores some internal state, so calling this function in a loop will
        // yield different seeds each time (unless srand() is called before this function)
        uint16_t seed = rand();
        // Write phase: Immediately check that the correct value was stored
        srand(seed);
        for (uint16_t i = 0; i < ext_ram_size; i++) {
            uint8_t some_value = rand();
            ext_ram[i] = some_value;
            uint8_t retreived_value = ext_ram[i];
            if (retreived_value != some_value) {
                printf("Write phase error: ext_ram[%4d] = %02X (should be %02X)\r\n", i, retreived_value, some_value);
                write_errors++;
            }
        }
        // Retrieval phase: Check that no values were changed during or after the write phase
        srand(seed);
        // reset the PRNG to the state it had before the write phase
        for (uint16_t i = 0; i < ext_ram_size; i++) {
            uint8_t some_value = rand();
            uint8_t retreived_value = ext_ram[i];
            if (retreived_value != some_value) {
                printf("Retrieval phase error: ext_ram[%4d] = %02X (should be %02X)\r\n", i, retreived_value, some_value);
                retrieval_errors++;
            }
        }
        printf("SRAM test completed with \r\n%4d errors in write phase and \r\n%4d errors in retrieval phase\r\n\r\n", write_errors, retrieval_errors);
}


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



void lets_test_the_fucking_adc(void) {
    uint8_t value;

    *ADC_ADDR = 0x55;
    value = *ADC_ADDR;
    printf("ADC saved 0x55, read 0x%2X\r\n", value);
    *ADC_ADDR = 0xAA;
    value = *ADC_ADDR;
    printf("ADC saved 0xAA, read 0x%2X\r\n", value);
}

void adc_init(void) {
    *ADC_ADDR = 0x91;
    *ADC_ADDR = 0x81;
    _delay_us(30);
    volatile uint8_t result = *ADC_ADDR;
    (void)result;
}


int main() {
    uart1_init();
    int theTime = 250;
    unsigned char recieved_text[10];
    int count = 0;
    fdevopen(uart_sendltr, uart_getltr);
    enable_external_sram();
    SRAM_test();
    //lets_test_the_fucking_ram();
    //lets_test_the_fucking_adc();
    adc_clock_init();
    joystick_calibration();
    while (1) {
        /*for (uint8_t i = 0; i < 4; i++) {
            uint8_t value = adc_read(i);
            uint16_t millivolts = ((uint32_t)value * 2500UL) / 256UL;
            printf("Read value from adc: %u (so approx: %u)\r\n", value, millivolts);
        }
        printf("\r\n");*/
        

        // JOYSTICK
        int16_t x = joystick_to_percent(joystickX_read());
        int16_t y = joystick_to_percent(joystickY_read());
        printf("Joystick positions, X: %d Y: %d :)\r\n", x, y);

        //lets_test_the_fucking_ram();
        _delay_ms(50);
    }
    //lets_test_the_fucking_ram();

    
    //while(1){
        //    dummy = *((volatile uint8_t *) 0x1800);
    //    _delay_ms(20);
        //_delay_ms(1000);
        //dummy = *adc;
        //_delay_ms(1000);
    //}


    //SRAM_test();
    //initializeLedShow();
    //latchInit();
    /*while (1) {
        DDRA = 0xFF; // all a outs on.
        DDRB |= (1 << PB1);
        PORTB &= ~(1 << PB1);
        
        _delay_ms(1000);

        PORTB |= (1 << PB1); // high so its transparent
        PORTA = 0b01010101;
        PORTB &= ~(1 << PB1);

        _delay_ms(1000);
        PORTB |= (1 << PB1); // high so its transparent
        PORTA = 0b10101010;
        PORTB &= ~(1 << PB1);

        _delay_ms(1000);
        PORTB |= (1 << PB1); // high so its transparent
        PORTA = 0x00;
        PORTB &= ~(1 << PB1);

        _delay_ms(10000);
        //DDRB != (1 << PB1);
        //PORTB &= ~(1 << PB1);
        //PORTA = 0b01010101;
        //PORTB &= ~(1 << PB1);
        

    }*/
    /*while (1) {
        unsigned char recieved = uart_getltr();
        

        
        if (recieved == '\r' || count > 10) {
            for (int i = 0; i < 10; i++) {
                uart_sendltr(recieved_text[i]);
            }
            uart_sendltr('!');
            cleanArray(recieved_text);
            count = 0;
        } else {
            count += 1;
            if (recieved == '^?' || recieved == '^H') { // IF WE BACKSPACE IN TERMINAL, REMOVE THE LAST INPUT
                recieved_text[count-1] = 'U';
            } else {
                recieved_text[count] = recieved;
            }
        }
        
        
    }*/

    return 1;
}



// UART sends least significant bit first
// 'A' = 0x41 = 01000001
// 0 10000010 1
// start data stop (no parity)

