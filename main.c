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
#include <stdbool.h>

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
    //oled_init();

    sei();
    while (1) {
        /*Touch Pad
        spi_select_slave(1);
        uint8_t data_pad = spi_write_read(0x01);       
        _delay_us(60);
        uint8_t x_pad = spi_write_read(0);
        _delay_us(2);   
        uint8_t y_pad = spi_write_read(0);
        _delay_us(2);   
        uint8_t size_pad = spi_write_read(0);
        printf("X, Y, Size: %d, %d, %d\n\r", x_pad, y_pad, size_pad);

        // Touch Slider
        spi_select_slave(1);
        uint8_t data_slider = spi_write_read(0x02);       
        _delay_us(60);
        uint8_t x_slider = spi_write_read(0);
        _delay_us(2);   
        uint8_t size_slider = spi_write_read(0);
        printf("X, Size: %d, %d\n\r", x_slider, size_slider);*/

        //Joystick
        spi_select_slave(1);
        uint8_t data_joy = spi_write_read(0x03);       
        _delay_us(60);
        uint8_t x_joy = spi_write_read(0);
        _delay_us(2);   
        uint8_t y_joy = spi_write_read(0);
        _delay_us(2);   
        uint8_t btn_joy = spi_write_read(0);
        printf("X, Y, Size: %d, %d, %d\n\r", x_joy, y_joy, btn_joy);
        

             
        /*
        data = spi_write_read(0x05);
        _delay_us(60);
        spi_write_read(5);
        spi_write_read(0);
        _delay_us(2);

        spi_write_read(0);
        spi_select_slave(0);
        _delay_ms(100);*/
    }


    

    /*
    oled_pos(0,0);
    oled_print("Hello!");

    oled_pos(2, 0);
    oled_print("-----");

    oled_pos(0,8);
    oled_print("morlb");

    oled_pos(0,0);
    oled_clear();
    oled_box(64,32, 126, 62, false);
    oled_box(22, 12, 41, 20, true);
    oled_box(64, 12, 41, 20, false);
    oled_box(106, 12, 41, 20, false);
    oled_pos(1,1);
    oled_print("Home");
    oled_pos(1,8);
    oled_print("SRAM");

    oled_pos(1,15);
    oled_print("DEMO");

    while (1) 
    {
        oled_task();
    }*/

    return 0;
}

