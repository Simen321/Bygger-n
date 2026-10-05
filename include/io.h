#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t size;
} Touchpad;

typedef struct {
    uint8_t x;
    uint8_t size;
} Slider;
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t btn;
} Joystick;
typedef struct __attribute__((packed)) {
    union {
        uint8_t right;
        struct {
            uint8_t R1:1;
            uint8_t R2:1;
            uint8_t R3:1;
            uint8_t R4:1;
            uint8_t R5:1;
            uint8_t R6:1;
        };
    };
    union {
        uint8_t left;
        struct {
            uint8_t L1:1;
            uint8_t L2:1;
            uint8_t L3:1;
            uint8_t L4:1;
            uint8_t L5:1;
            uint8_t L6:1;
            uint8_t L7:1;
        };
    };
    union {
        uint8_t nav;
        struct {
            uint8_t NB:1;
            uint8_t NR:1;
            uint8_t ND:1;
            uint8_t NL:1;
            uint8_t NU:1;
        };
    };
} Buttons;
typedef struct {
    char timestamp[19];
    uint8_t serial_number[16];
} Info;


Touchpad io_read_touchpad();
Slider io_read_slider();
Joystick io_read_joystick();
Buttons io_read_buttons();
Info io_read_info();



void io_led_on(uint8_t idx);
void io_led_off(uint8_t idx);
void io_led_toggle(uint8_t idx);
void io_led_increment();
void ui_led_decrease();

void io_print_all();
#endif