#define F_CPU 4915200UL
#define BAUD 9600UL
#define UBRR_VAL ((F_CPU) / (16 * BAUD) - 1) // teh baudrate formula

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

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
    UDR1 = c;

    return 0;
}

int uart_getltr(FILE *stream) {
    while (!(UCSR1A & (1 << RXC1)));
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

int main() {
    uart1_init();
    
    unsigned char recieved_text[10];
    int count = 0;
    fdevopen(uart_sendltr, uart_getltr);
    while (1){
        //scanf("Skriv inn noe commando: %s", recieved_text);
        printf(recieved_text);

        
        _delay_ms(1000);
    }
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

