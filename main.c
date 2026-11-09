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
#include "include/console.h"
#include "include/can_ids.h"
#include <avr/pgmspace.h>

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
    console_log_P(PSTR("node 1 started"));

    uint16_t rx_id = 0;
    uint8_t rx_data[8] = {0};
    uint8_t rx_len = 0;

    uint32_t last_tick = oled_ticks();
    uint8_t previous_r2 = 0;
    bool can_ok = true;

    while (1) {
        // empty both CAN receive buffers (game over, console text from node 2).
        // done every loop, not just every tick, since the mcp2515 only holds two frames.
        while (mcp_receive_can_message(&rx_id, rx_data, &rx_len)) {
            menu_can_message(rx_id, rx_data, rx_len);
        }

        // the rest runs once per oled refresh tick (60 Hz)
        uint32_t now = oled_ticks();
        if (now == last_tick) {
            continue;
        }
        last_tick = now;

        Joystick joystickdata = io_read_joystick();
        Slider sliderdata = io_read_slider();
        Touchpad touchpaddata = io_read_touchpad();
        Buttons buttondata = io_read_buttons();

        // inputs to node 2
        uint8_t joy_data[3] = {joystickdata.x, joystickdata.y, joystickdata.btn};
        uint8_t slider_touch_packet[3] = {sliderdata.x, touchpaddata.x, touchpaddata.y};
        uint8_t button_packet[1] = {buttondata.R2};
        bool sent = mcp_send_can_message(CAN_ID_JOYSTICK, joy_data, 3);
        sent = mcp_send_can_message(CAN_ID_SLIDER, slider_touch_packet, 3) && sent;
        sent = mcp_send_can_message(CAN_ID_BUTTONS, button_packet, 1) && sent;

        // R2 fires the solenoid once per press, not every tick while held
        if (buttondata.R2 && !previous_r2) {
            mcp_send_can_message(CAN_ID_SOLENOID, button_packet, 1);
        }
        previous_r2 = buttondata.R2;

        // tell on the console when node 2 stops or starts answering
        if (sent != can_ok) {
            can_ok = sent;
            if (sent) {
                console_log_P(PSTR("CAN: node 2 back"));
            } else {
                console_log_P(PSTR("CAN: node 2 no answer"));
            }
        }

        menu_update(buttondata, now);
        menu_draw(now);
        oled_update();
    }
    return 0;
}

