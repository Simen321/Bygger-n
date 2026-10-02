#ifndef F_CPU
#define F_CPU 4915200UL
#endif

#include "../include/oled.h"
#include "../include/spi.h"
#include "../labtools/fonts.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


// using the external sram as the framebuffer, get 1kb from 0x1800 -> 0x1BFF
static volatile uint8_t * const oled_buffer = (volatile uint8_t *)OLED_FRAMEBUFFER_ADDR;

static uint8_t cursor_page = 0;
static uint8_t cursor_column = 0;

static volatile uint8_t oled_refreshrate_pending = 0;


static void oled_timer_init(void)
{
    TCCR1A = 0;

    // ctc mode, prescaler = 64
    TCCR1B =
        (1 << WGM12) |
        (1 << CS11) |
        (1 << CS10);

    OCR1A =
        (F_CPU / (64UL * OLED_REFRESH_RATE)) - 1;

    TIMSK |= (1 << OCIE1A);
}


ISR(TIMER1_COMPA_vect)
{
    oled_refreshrate_pending = 1;
}


// 64 rows (height) and 128 columns (width)

static void oled_write_command(uint8_t command)
{
    OLED_CS_LOW();
    OLED_DC_LOW();

    spi_transfer(command);

    OLED_CS_HIGH();
}


static void oled_write_data(uint8_t data)
{
    OLED_CS_LOW();
    OLED_DC_HIGH();

    spi_transfer(data);

    OLED_CS_HIGH();
}


void oled_reset(void)
{
    OLED_RST_LOW();
    _delay_ms(10);

    OLED_RST_HIGH();
    _delay_ms(10);
}


void oled_init(void)
{
    OLED_CS_DDR  |= (1 << OLED_CS_PIN);
    OLED_DC_DDR  |= (1 << OLED_DC_PIN);
    OLED_RST_DDR |= (1 << OLED_RST_PIN);

    OLED_CS_HIGH();
    OLED_DC_HIGH();
    OLED_RST_HIGH();

    oled_reset();

    // display off
    oled_write_command(0xAE);

    // multiplex ratio: 64 rows
    oled_write_command(0xA8);
    oled_write_command(0x3F);

    // display offset
    oled_write_command(0xD3);
    oled_write_command(0x00);

    // page addressing mode
    oled_write_command(0x20);
    oled_write_command(0x02);

    // display start line
    oled_write_command(0x40);

    // normal display
    oled_write_command(0xA6);

    // contrast
    oled_write_command(0x81);
    oled_write_command(0x6F);

    // display clock
    oled_write_command(0xD5);
    oled_write_command(0xC0);

    // segment remap
    oled_write_command(0xA1);

    // com scan direction
    oled_write_command(0xC8);

    // com configuration
    oled_write_command(0xDA);
    oled_write_command(0x12);

    // precharge
    oled_write_command(0xD9);
    oled_write_command(0xF1);

    // vcomh
    oled_write_command(0xDB);
    oled_write_command(0x3C);

    // display follows ram
    oled_write_command(0xA4);

    // display on
    oled_write_command(0xAF);

    oled_clear();
    oled_update();

    oled_timer_init();
}


void oled_update(void)
{
    for (uint8_t page = 0; page < OLED_PAGENR; page++)
    {
        oled_goto_page(page);
        oled_goto_column(0);

        uint16_t offset = (uint16_t)page * OLED_WIDTH;

        OLED_CS_LOW();
        OLED_DC_HIGH();

        for (uint8_t column = 0; column < OLED_WIDTH; column++)
        {
            spi_transfer(oled_buffer[offset + column]);
        }

        OLED_CS_HIGH();
    }
}


void oled_task(void)
{
    if (oled_refreshrate_pending)
    {
        oled_refreshrate_pending = 0;
        oled_update();
    }
}


void oled_goto_page(uint8_t page)
{
    if (page >= OLED_PAGENR){
        return;
    }

    oled_write_command(0xB0 | page);
}


void oled_goto_column(uint8_t column)
{
    if (column >= OLED_WIDTH){
        return;
    }

    oled_write_command(0x00 | (column & 0x0F));
    oled_write_command(0x10 | ((column >> 4) & 0x0F));
}


void oled_home(void)
{
    cursor_page = 0;
    cursor_column = 0;
}


void oled_goto_line(uint8_t line)
{
    if (line >= OLED_PAGENR){
        return;
    }

    cursor_page = line;
    cursor_column = 0;
}


void oled_clear(void)
{
    for (uint16_t i = 0; i < OLED_FRAMEBUFFER_SIZE; i++)
    {
        oled_buffer[i] = 0x00;
    }

    oled_home();
}


void oled_clear_line(uint8_t line)
{
    if (line >= OLED_PAGENR){
        return;
    }

    uint16_t offset = (uint16_t)line * OLED_WIDTH;

    for (uint8_t column = 0; column < OLED_WIDTH; column++)
    {
        oled_buffer[offset + column] = 0x00;
    }

    cursor_page = line;
    cursor_column = 0;
}


void oled_pos(uint8_t row, uint8_t column)
{
    if (row >= OLED_PAGENR){
        return;
    }

    uint16_t pixel_column = (uint16_t)column * OLED_CHAR_WIDTH;

    if (pixel_column >= OLED_WIDTH){
        return;
    }

    cursor_page = row;
    cursor_column = (uint8_t)pixel_column;
}


void oled_putchar(char c)
{
    if (c < 32 || c > 126){
        c = '?';
    }

    if (cursor_page >= OLED_PAGENR){
        return;
    }

    if ((uint16_t)cursor_column + OLED_CHAR_WIDTH > OLED_WIDTH){
        return;
    }

    uint8_t font_index = c - 32;

    uint16_t buffer_index =
        ((uint16_t)cursor_page * OLED_WIDTH)
        + cursor_column;

    for (uint8_t i = 0; i < OLED_FONT_WIDTH; i++)
    {
        oled_buffer[buffer_index + i] =
            pgm_read_byte(&font5[font_index][i]);
    }

    oled_buffer[buffer_index + OLED_FONT_WIDTH] = 0x00;

    cursor_column += OLED_CHAR_WIDTH;
}


void oled_print(const char *str)
{
    while (*str)
    {
        oled_putchar(*str++);
    }
}