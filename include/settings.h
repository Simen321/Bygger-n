#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

// motor settings edited in the settings menu and sent to node 2.
// they are sent when they change and every SETTINGS_RESEND_SECONDS, so node 2 gets them
// again after a restart. start values match the defaults in node 2's motor.c.

#define SETTINGS_RESEND_SECONDS 2

typedef struct {
    uint8_t scheme;         // CONTROL_POSITION or CONTROL_VELOCITY (can_ids.h)
    uint16_t max_speed;     // highest PWM duty node 2 may use, of 4200
    uint16_t pid[3];        // p, i, d in hundredths (5.00 -> 500)
} Settings;

extern Settings settings;

void settings_send(void);           // send now (call after a change)
void settings_task(uint32_t now);   // call every tick, resends now and then

#endif
