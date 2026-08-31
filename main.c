#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

int main() {
    DDRB |= (1 << DDB0); // skriv 1 til bittn for å sette den til output


    // også loope med å sette outputn til pinnen 1 eller 0
    while (1) {
        PORTB |= (1 << PB0);  
        _delay_ms(1000);

        PORTB &= ~(1 << PB0);
        _delay_ms(1000);
    }
    return 0;
}