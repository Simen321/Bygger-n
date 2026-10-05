#include "../include/io.h"
#include "../include/spi.h"

#define DELAY_TIME 60

Touchpad io_read_touchpad()
{
    spi_select_slave(1);

    uint8_t result = spi_write_read(0x01);
    _delay_us(DELAY_TIME);
    uint8_t x = spi_write_read(0);
    _delay_us(2);
    uint8_t y = spi_write_read(0);
    _delay_us(2);
    uint8_t size = spi_write_read(0);

    spi_select_slave(0);

    Touchpad touchpad = {
        .x = x,
        .y = y,
        .size = size
    };

    return touchpad;
};

Slider io_read_slider()
{
    spi_select_slave(1);

    uint8_t data_slider = spi_write_read(0x02);       
    _delay_us(DELAY_TIME);
    uint8_t x = spi_write_read(0);
    _delay_us(2);   
    uint8_t size = spi_write_read(0);

    spi_select_slave(0);

    Slider slider = {
        .x = x,
        .size = size
    };
    return slider;
};

Joystick io_read_joystick()
{
    spi_select_slave(1);

    uint8_t data_joy = spi_write_read(0x03);       
    _delay_us(DELAY_TIME);
    uint8_t x = spi_write_read(0);
    _delay_us(2);   
    uint8_t y = spi_write_read(0);
    _delay_us(2);   
    uint8_t btn = spi_write_read(0);

    spi_select_slave(0);
    Joystick joystick = {
        .x = x,
        .y = y,
        .btn = btn
    };
    return joystick;
};
Buttons io_read_buttons()
{
    Buttons status;
    spi_select_slave(1);

    uint8_t data = spi_write_read(0x04);
    _delay_us(DELAY_TIME);
    status.right = spi_write_read(0);
    _delay_us(2);
    status.left = spi_write_read(0);
    _delay_us(2);
    status.nav = spi_write_read(0);

    spi_select_slave(0);
    return status;

};
Info io_read_info()
{
    spi_select_slave(1);
    Info info;
    uint8_t data = spi_write_read(0x07);
    _delay_ms(DELAY_TIME);
    for (uint8_t i = 0; i < 19; i++) {
        uint8_t curByte = spi_write_read(0);
        _delay_us(2);
        info.timestamp[i] = curByte;
    }

    for (uint8_t i = 19; i < 35; i++){
        uint8_t curByte = spi_write_read(0);
        _delay_us(2);
        info.serial_number[i] = curByte;
    }
    spi_select_slave(0);

    return info;
};

void io_led_on(uint8_t idx);
void io_led_off(uint8_t idx);
void io_led_toggle(uint8_t idx);
void io_led_increment();
void ui_led_decrease();
