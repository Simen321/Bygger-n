#include "../include/uart.h"
#include <avr/io.h>

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



void cleanArray(unsigned char arr[10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = 0;
    }
}
