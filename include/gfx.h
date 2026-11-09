#ifndef GFX_H
#define GFX_H

#include <stdint.h>

// pixel drawing into the oled framebuffer. call oled_update() to show it.
// x/y are pixels, (0, 0) is the top left corner. anything outside the screen is clipped.

#define GFX_SET    0 // turn pixels on
#define GFX_CLEAR  1 // turn pixels off
#define GFX_INVERT 2 // flip pixels

#define GFX_CHAR_HEIGHT 7 // font5 uses 7 of the 8 rows

// width in pixels of a text of `len` characters (without the blank column after the last one)
#define GFX_TEXT_WIDTH(len, scale) ((int16_t)(len) * 6 * (scale) - (scale))

void gfx_pixel(int16_t x, int16_t y, uint8_t mode);
void gfx_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t mode);
void gfx_rect(int16_t x, int16_t y, int16_t w, int16_t h); // 1 px outline

// text with font5, scale 1 = 5x7 pixels per character, scale 2 = 10x14 ...
void gfx_text(int16_t x, int16_t y, const char *str, uint8_t scale, uint8_t mode);
void gfx_text_P(int16_t x, int16_t y, const char *str, uint8_t scale, uint8_t mode); // str in flash (PSTR)

// same, but centered horizontally on the screen
void gfx_text_centered(int16_t y, const char *str, uint8_t scale, uint8_t mode);
void gfx_text_centered_P(int16_t y, const char *str, uint8_t scale, uint8_t mode);

#endif
