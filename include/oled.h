#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include "../labtools/fonts.h"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGENR 8
#define OLED_REFRESH_RATE 60
#define OLED_FRAMEBUFFER_ADDR 0x1800
#define OLED_FRAMEBUFFER_SIZE 1024

#define OLED_FONT_WIDTH 5
#define OLED_CHAR_WIDTH 6


// config pinsetup and helpers
#define OLED_CS_DDR     //DDRD
#define OLED_CS_PORT    //PORTD
#define OLED_CS_PIN     //PD0

#define OLED_DC_DDR     //DDRD
#define OLED_DC_PORT    //PORTD
#define OLED_DC_PIN     //PD1

#define OLED_RST_DDR    //DDRD
#define OLED_RST_PORT   //PORTD
#define OLED_RST_PIN    //PD2


#define OLED_CS_LOW()   (OLED_CS_PORT &= ~(1 << OLED_CS_PIN))
#define OLED_CS_HIGH()  (OLED_CS_PORT |=  (1 << OLED_CS_PIN))

#define OLED_DC_LOW()   (OLED_DC_PORT &= ~(1 << OLED_DC_PIN))
#define OLED_DC_HIGH()  (OLED_DC_PORT |=  (1 << OLED_DC_PIN))

#define OLED_RST_LOW()  (OLED_RST_PORT &= ~(1 << OLED_RST_PIN))
#define OLED_RST_HIGH() (OLED_RST_PORT |=  (1 << OLED_RST_PIN))




void oled_init(void);
void oled_reset(void);
void oled_home(void);
void oled_goto_line(uint8_t line);
void oled_goto_column(uint8_t column);

void oled_clear(void);
void oled_clear_line(uint8_t line);
void oled_pos(uint8_t row, uint8_t column);

void oled_putchar(char c);
void oled_print(const char *str);


// optional
//void oled_line(x0,y0,x1,y1);
//void oled_circle(x,y,r);

// Changing things around to try to make contineous refresh rate work
void oled_update(void);
void oled_task(void);


#endif