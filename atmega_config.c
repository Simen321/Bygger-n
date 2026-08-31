#define F_CPU 4915200UL
#define BAUD 9600UL
#define UBRR_VAL ((F_CPU) / (16 * BAUD) - 1) // teh baudrate formula

#include <avr/io.h>
#include <util/delay.h>

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
void uart1_putchar(char c) {
    while (!(UCSR1A & (1 << UDRE1)));
    UDR1 = c;
}

unsigned char uart1_getchar(void) {
    while (!(UCSR1A & (1 << RXC1)));
    unsigned char recieveed_data = UDR1; // UDR1 er da dataen bare, samme som i transmitt. Derfor er det avhengig av timingen.
    return recieveed_data;
}




int main() {
    uart1_init();
    unsigned char recieved_text[10];
    int count = 0;
    while (1) {
        unsigned char recieved = uart1_getchar();
        
        
        if (recieved == '\r' | count > 10) {
            for (int i = 0; i < 10; i++) {
                uart1_putchar(recieved_text[i]);
            }
            uart1_putchar('!');
            count = 0;
        } else {
            count += 1;
            recieved_text[count] = recieved;
        }
        
        
    }

    return 1;
}



// UART sends least significant bit first
// 'A' = 0x41 = 01000001
// 0 10000010 1
// start data stop (no parity)

