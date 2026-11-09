#include "../include/settings.h"
#include "../include/can_ids.h"
#include "../include/MCP2515.h"
#include "../include/oled.h"

Settings settings = {
    .scheme = CONTROL_POSITION,
    .max_speed = 2500,
    .pid = {500, 100, 25}, // 5.00, 1.00, 0.25
};

static uint32_t last_sent = 0;
static uint8_t sent_once = 0;


void settings_send(void)
{
    uint8_t control[3] = {
        settings.scheme,
        settings.max_speed >> 8,
        settings.max_speed & 0xFF,
    };
    mcp_send_can_message(CAN_ID_SETTINGS_CONTROL, control, 3);

    uint8_t pid[6];
    for (uint8_t i = 0; i < 3; i++) {
        pid[2 * i] = settings.pid[i] >> 8;
        pid[2 * i + 1] = settings.pid[i] & 0xFF;
    }
    mcp_send_can_message(CAN_ID_SETTINGS_PID, pid, 6);

    last_sent = oled_ticks();
    sent_once = 1;
}


void settings_task(uint32_t now)
{
    if (!sent_once || now - last_sent >= SETTINGS_RESEND_SECONDS * OLED_TICKS_PER_SECOND) {
        settings_send();
    }
}
