#ifndef JOYSTICK_H
#define JOYSTICK_H
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

uint8_t joystickX_read();
uint8_t joystickY_read();
int16_t joystick_to_percent(uint8_t input);

void printJoystickXY();

void joystick_calibration();

#endif