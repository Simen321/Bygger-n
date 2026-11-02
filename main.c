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
#include "include/io.h"
#include "include/menu.h"
#include "include/MCP2515.h"

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

// Vår eksterne SRAM er fra 0x1000 til 0x1C00, som er da en størrelse på 3kb fra 0x1000. SRAM testen kjører da 3kb fra 0x1000.

//FILE oled_stream = FDEV_SETUP_STREAM(oled_putchar_stream, NULL, _FDEV_SETUP_WRITE);
//FILE uart_stream = FDEV_SETUP_STREAM(uart_sendltr, uart_getltr, _FDEV_SETUP_RW);



int main(void) {
    uart1_init();
    unsigned char recieved_text[10];
    int count = 0;
    fdevopen(uart_sendltr, uart_getltr);
    _delay_ms(1000);
    //printf("Booted..\n\r");

    enable_external_sram();
    SRAM_test();

    adc_clock_init();

    // Main program should from now on be controlled thorugh the user interface on the OLED and navigation through joystick and touchscreens
    // OLED:
    spi_init();
    oled_init();

    sei();
    
    mcp_init(NORMAL_MODE);
    const uint8_t data[5] = {6,7,6,7,67};
    //mcp_send_can_message(0x101,data, 5);
    
    uint16_t rx_id = 0;
    uint8_t rx_data[8] = {0};
    uint8_t rx_len = 0;

    uint16_t timeout = 0;

    
    while (1) {
        Joystick joystickdata = io_read_joystick();
        uint8_t joy_data[3] = {joystickdata.x, joystickdata.y, joystickdata.btn};
        mcp_send_can_message(0x101, joy_data, 3);

        // slider and touchpad
        Slider sliderdata = io_read_slider();
        Touchpad touchpaddata = io_read_touchpad();
        uint8_t slider_touch_packet[3] = {sliderdata.x, touchpaddata.x, touchpaddata.y};
        mcp_send_can_message(0x102, slider_touch_packet, 3);

        // buttons
        Buttons buttondata = io_read_buttons();
        uint8_t button_packet[1] = {buttondata.R2};
        mcp_send_can_message(0x103, button_packet, 1);


        
        

        ////printf("CANSTAT: 0x%02X\r\n", mcp_read(0x0E));
        ////printf("TXB0CTRL: 0x%02X\r\n", mcp_read(0x30));
        ////printf("CANINTF: 0x%02X\r\n", mcp_read(0x2C));
        ////printf("EFLG:    0x%02X\r\n", mcp_read(0x2D));
        ////printf("CANCTRL: 0x%02X\r\n", mcp_read(0x0F));
        //io_print_all();


        Buttons btns = io_read_buttons();
        if (btns.NR) {
            menu_move_right();
        }
        if (btns.NL) {
            menu_move_left();
        }
        if (btns.NB) {
            menu_click();
        }
        
        if (btns.NU) {
            menu_move_up();
        }
        if (btns.ND) {
            menu_move_down();
        }
        if (btns.R2) {
            uint8_t signal[1] = {1};
            mcp_send_can_message(0x100, signal, 1);
        }

        /*
        if (mcp_receive_can_message(&rx_id, rx_data, &rx_len)) {
            printf("recieved id  %d\n\r", rx_id);
            printf("length %d\n\r", rx_len);
            for (uint8_t i = 0; i < rx_len && i < 8; i++) {
                printf(" %u", (unsigned int)rx_data[i]);
            }
        }
        */
        //printf("aftr btn");
        uint8_t idx = menu_get_main_index();
        ////printf("Main menu index: %d\n\r", idx);
        update_menu_visuals();
        //printf("ran oled loop");
        oled_task();
        //mcp_send_can_message(0x101,data, 5);



        //_delay_ms(16);
    }
    return 0;
}

