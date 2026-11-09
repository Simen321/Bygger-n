#include "../include/gfx.h"
#include "../include/oled.h"
#include "../labtools/fonts.h"

#include <avr/pgmspace.h>
#include <string.h>

// the framebuffer is 8 pages of 128 bytes. each byte is a column of 8 pixels, bit 0 at the top.
// working on whole bytes instead of single pixels is much faster, which matters since the
// framebuffer is in external sram and the whole screen is redrawn 60 times a second.

static void apply(volatile uint8_t *byte, uint8_t mask, uint8_t mode)
{
    if (mode == GFX_SET) {
        *byte |= mask;
    } else if (mode == GFX_CLEAR) {
        *byte &= ~mask;
    } else {
        *byte ^= mask;
    }
}


void gfx_pixel(int16_t x, int16_t y, uint8_t mode)
{
    if (x < 0 || y < 0 || x >= OLED_WIDTH || y >= OLED_HEIGHT) {
        return;
    }
    apply(&oled_framebuffer()[(y >> 3) * OLED_WIDTH + x], 1 << (y & 7), mode);
}


void gfx_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t mode)
{
    // clip to the screen
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > OLED_WIDTH)  { w = OLED_WIDTH - x; }
    if (y + h > OLED_HEIGHT) { h = OLED_HEIGHT - y; }
    if (w <= 0 || h <= 0) {
        return;
    }

    volatile uint8_t *fb = oled_framebuffer();
    int16_t y_end = y + h; // first row below the rect

    for (int16_t page = y >> 3; page <= (y_end - 1) >> 3; page++) {
        // which of the 8 rows in this page the rect covers
        int16_t top = page * 8;
        uint8_t mask = 0xFF;
        if (y > top) {
            mask &= (uint8_t)(0xFF << (y - top));
        }
        if (y_end < top + 8) {
            mask &= (uint8_t)(0xFF >> (top + 8 - y_end));
        }

        volatile uint8_t *row = fb + page * OLED_WIDTH + x;
        for (int16_t i = 0; i < w; i++) {
            apply(row + i, mask, mode);
        }
    }
}


void gfx_rect(int16_t x, int16_t y, int16_t w, int16_t h)
{
    gfx_fill_rect(x, y, w, 1, GFX_SET);         // top
    gfx_fill_rect(x, y + h - 1, w, 1, GFX_SET); // bottom
    gfx_fill_rect(x, y, 1, h, GFX_SET);         // left
    gfx_fill_rect(x + w - 1, y, 1, h, GFX_SET); // right
}


static void draw_char(int16_t x, int16_t y, char c, uint8_t scale, uint8_t mode)
{
    if (c < 32 || c > 126) {
        c = '?';
    }

    for (uint8_t col = 0; col < OLED_FONT_WIDTH; col++) {
        uint8_t bits = pgm_read_byte(&font5[c - 32][col]);
        int16_t cx = x + col * scale;

        if (scale == 1 && y >= 0) {
            // fast path: the column is one byte, shifted into the one or two pages it covers
            if (cx < 0 || cx >= OLED_WIDTH) {
                continue;
            }
            volatile uint8_t *fb = oled_framebuffer();
            uint8_t page = y >> 3;
            uint8_t shift = y & 7;
            if (page < OLED_PAGENR) {
                apply(&fb[page * OLED_WIDTH + cx], (uint8_t)(bits << shift), mode);
            }
            if (shift && page + 1 < OLED_PAGENR) {
                apply(&fb[(page + 1) * OLED_WIDTH + cx], bits >> (8 - shift), mode);
            }
        } else {
            // scaled: every font pixel becomes a scale x scale block
            for (uint8_t row = 0; row < 8; row++) {
                if (bits & (1 << row)) {
                    gfx_fill_rect(cx, y + row * scale, scale, scale, mode);
                }
            }
        }
    }
}


void gfx_text(int16_t x, int16_t y, const char *str, uint8_t scale, uint8_t mode)
{
    while (*str) {
        draw_char(x, y, *str++, scale, mode);
        x += 6 * scale;
    }
}


void gfx_text_P(int16_t x, int16_t y, const char *str, uint8_t scale, uint8_t mode)
{
    char c;
    while ((c = pgm_read_byte(str++))) {
        draw_char(x, y, c, scale, mode);
        x += 6 * scale;
    }
}


void gfx_text_centered(int16_t y, const char *str, uint8_t scale, uint8_t mode)
{
    gfx_text((OLED_WIDTH - GFX_TEXT_WIDTH(strlen(str), scale)) / 2, y, str, scale, mode);
}


void gfx_text_centered_P(int16_t y, const char *str, uint8_t scale, uint8_t mode)
{
    gfx_text_P((OLED_WIDTH - GFX_TEXT_WIDTH(strlen_P(str), scale)) / 2, y, str, scale, mode);
}
