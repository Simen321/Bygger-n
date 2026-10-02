#define F_CPU 4915200UL
#define BAUD 9600UL
#define UBRR_VAL ((F_CPU) / (16 * BAUD) - 1) // teh baudrate formula

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>

#include "labtools/sram_test_cr.h"
#include "include/adc.h"
#include "include/joystick.h"
#include "include/spi.h"
#include "include/oled.h"
#include "include/uart.h"

#include <math.h>

int sram_address_min = 0x000;
int sram_address_max = 0xBFF;
int dac_address_min = 0xC00;
int dac_address_max = 0xFFF;

void enable_external_sram(void) { // stolen directly from google after irritation <3
    // Set the SRE bit to enable the external memory interface
    //MCUCR |= (1 <uart1_init< SRE);
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





int main(void) {
    uart1_init();
    unsigned char recieved_text[10];
    int count = 0;
    fdevopen(uart_sendltr, uart_getltr);
    
    enable_external_sram();
    SRAM_test();

    adc_clock_init();

    /*joystick_calibration();
    while (1) {
        // adc_print_all();
        // JOYSTICK
        int16_t x = joystick_to_percent(joystickX_read());
        int16_t y = joystick_to_percent(joystickY_read());
        printf("Joystick positions, X: %d Y: %d :)\r\n", x, y);

        _delay_ms(50);
    }*/
    /*
    spi_init();
    while (1) {
        spi_test();
        _delay_ms(100);
    }*/

    /*
    uart1_init();
    unsigned char recieved_text[10];
    int count = 0;
    fdevopen(uart_sendltr, uart_getltr);
    
    enable_external_sram();
    SRAM_test();

    adc_clock_init();
*/
    // Main program should from now on be controlled thorugh the user interface on the OLED and navigation through joystick and touchscreens
    // OLED:
    spi_init();
    oled_init();

    sei();

    oled_pos(0,0);
    oled_print("Hello!");

    oled_pos(2, 0);
    oled_print("-----");

    oled_pos(0,8);
    oled_print("morlb");
    // Sentrum for banen (midten av OLED-skjermen: x=64, y=32)
    uint8_t center_x = 64;
    uint8_t center_y = 32;
    
    // Radius på banen den skal rotere rundt
    uint8_t path_radius = 20; 
    
    // Radius på den faktiske sirkelen som tegnes/animeres
    uint8_t circle_radius = 4;

    // Vinkel i radianer (starter på 0)
    double angle = 0.0;

    while (1)
    {
        // 1. Tøm framebufferet i SRAM slik at den gamle sirkelen forsvinner (Double buffering)
        // (Hvis du har tekst du vil beholde, må den printes på nytt etter clear, 
        //  eller så må du kun viske ut den gamle sirkelen).
        oled_clear();
        
        // Siden oled_clear() sletter alt, repliterer vi teksten her:
        oled_pos(0, 0); oled_print("Hello!");
        oled_pos(2, 0); oled_print("-----");
        oled_pos(0, 8); oled_print("morlb");

        // 2. Valgfritt: Tegn selve "banen" sirkelen skal følge (hvis du vil se den)
        // oled_circle(center_x, center_y, path_radius);

        // 3. Beregn den nye posisjonen til den animerte sirkelen basert på vinkelen
        uint8_t anim_x = (uint8_t)(center_x + path_radius * cos(angle));
        uint8_t anim_y = (uint8_t)(center_y + path_radius * sin(angle));

        // 4. Tegn den animerte sirkelen på sin nye posisjon
        oled_circle(anim_x, anim_y, circle_radius);

        // 5. Øk vinkelen for neste bilderamme (jo høyere tall, jo raskere roterer den)
        angle += 0.15; 
        if (angle >= 2 * M_PI) {
            angle = 0.0; // Nullstill vinkel når den har gått hele veien rundt
        }

        // 6. Vent litt så animasjonen ikke blir et eneste stort blinkende flimmer
        _delay_ms(40);

        // 7. La timeren din dytte det nye bildet til skjermen
        oled_task();
    }

    return 0;
}

