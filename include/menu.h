#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include "io.h"

// menu on the oled. call once per screen refresh:
//   menu_update(buttons, now);  // handle the navigation buttons
//   menu_draw(now);             // draw into the framebuffer, then oled_update()
// `now` is oled_ticks().

void menu_update(Buttons buttons, uint32_t now);
void menu_draw(uint32_t now);

// pass every CAN frame from node 2 here (game over, console text)
void menu_can_message(uint16_t id, const uint8_t *data, uint8_t length);

// latest inputs and link state, shown on the sensors and info pages
void menu_set_inputs(Joystick joystick, Slider slider, Touchpad touchpad);
void menu_set_can_ok(uint8_t ok);

#endif
