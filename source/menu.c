#include "../include/menu.h"
#include "../include/gfx.h"
#include "../include/oled.h"
#include "../include/console.h"
#include "../include/can_ids.h"
#include "../include/MCP2515.h"
#include "../include/adc.h"

#include <avr/pgmspace.h>
#include <stdio.h>

// Screens:
//   HOME          Demo (big button), Info, Sensors, Console. up/down to move, click to open
//   DEMO_SETUP    player count (up/down, 1-4) and Start (click). left goes back home
//   GAME          full screen game, one round per player. hold left to exit to DEMO_SETUP
//   INFO/SENSORS/CONSOLE   left goes back home

typedef enum {
    SCREEN_HOME,
    SCREEN_DEMO_SETUP,
    SCREEN_GAME,
    SCREEN_INFO,
    SCREEN_SENSORS,
    SCREEN_CONSOLE,
} Screen;

typedef enum {
    GAME_READY,     // "player n" and a start button
    GAME_PLAYING,   // timer running until node 2 says the ball is lost
    GAME_OVER,      // time for this player, then next player or results
    GAME_RESULTS,   // all times, winner highlighted, restart
} GameState;

// home options, in the order they are shown
enum { HOME_DEMO, HOME_INFO, HOME_SENSORS, HOME_CONSOLE, HOME_COUNT };

#define MAX_PLAYERS 4

// exit the game by holding left: nothing happens for EXIT_DELAY, then a circle fills over EXIT_FILL
#define EXIT_DELAY (OLED_TICKS_PER_SECOND / 2)
#define EXIT_FILL  (OLED_TICKS_PER_SECOND * 3 / 2)

static Screen screen = SCREEN_HOME;
static uint8_t home_index = HOME_DEMO;
static uint8_t player_count = 1;

static GameState game_state = GAME_READY;
static uint8_t current_player = 0;
static uint32_t game_start_tick = 0;
static uint32_t player_time[MAX_PLAYERS]; // in ticks

static uint8_t previous_nav = 0;   // nav buttons last update, to find new presses
static uint8_t left_holding = 0;   // left is being held in the game
static uint32_t left_down_tick = 0;


// ---------------------------------------------------------------- input

static void send_can(uint16_t id)
{
    uint8_t nothing = 0;
    mcp_send_can_message(id, &nothing, 0);
}

static void game_open(void)
{
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
        player_time[i] = 0;
    }
    current_player = 0;
    game_state = GAME_READY;
    screen = SCREEN_GAME;
}

static void game_exit(void)
{
    if (game_state == GAME_PLAYING) {
        send_can(CAN_ID_GAME_STOP);
    }
    left_holding = 0;
    game_state = GAME_READY;
    screen = SCREEN_DEMO_SETUP;
}

static void game_update(Buttons pressed, Buttons held, uint32_t now)
{
    // hold left to exit
    if (held.NL) {
        if (!left_holding) {
            left_holding = 1;
            left_down_tick = now;
        } else if (now - left_down_tick >= EXIT_DELAY + EXIT_FILL) {
            game_exit();
            return;
        }
    } else {
        left_holding = 0;
    }

    if (!pressed.NB) {
        return;
    }

    switch (game_state) {
        case GAME_READY:
            send_can(CAN_ID_SOLENOID);   // serve the ball
            send_can(CAN_ID_GAME_START); // node 2 starts watching the ir beam
            game_start_tick = now;
            game_state = GAME_PLAYING;
            break;

        case GAME_PLAYING:
            break; // only node 2 can end a round

        case GAME_OVER:
            if (current_player + 1 < player_count) {
                current_player++;
                game_state = GAME_READY;
            } else {
                game_state = GAME_RESULTS;
            }
            break;

        case GAME_RESULTS:
            game_open(); // restart with the same players
            break;
    }
}

void menu_update(Buttons buttons, uint32_t now)
{
    // only react when a button goes down, not every update while it is held
    Buttons pressed = {0};
    pressed.nav = buttons.nav & ~previous_nav;
    previous_nav = buttons.nav;

    switch (screen) {
        case SCREEN_HOME:
            if (pressed.NU && home_index > 0) {
                home_index--;
            }
            if (pressed.ND && home_index < HOME_COUNT - 1) {
                home_index++;
            }
            if (pressed.NB) {
                switch (home_index) {
                    case HOME_DEMO:    screen = SCREEN_DEMO_SETUP; break;
                    case HOME_INFO:    screen = SCREEN_INFO;       break;
                    case HOME_SENSORS: screen = SCREEN_SENSORS;    break;
                    case HOME_CONSOLE: screen = SCREEN_CONSOLE;    break;
                }
            }
            break;

        case SCREEN_DEMO_SETUP:
            if (pressed.NU && player_count < MAX_PLAYERS) {
                player_count++;
            }
            if (pressed.ND && player_count > 1) {
                player_count--;
            }
            if (pressed.NB) {
                game_open();
            }
            if (pressed.NL) {
                screen = SCREEN_HOME;
            }
            break;

        case SCREEN_GAME:
            game_update(pressed, buttons, now);
            break;

        case SCREEN_INFO:
        case SCREEN_SENSORS:
        case SCREEN_CONSOLE:
            if (pressed.NL) {
                screen = SCREEN_HOME;
            }
            break;
    }
}

void menu_can_message(uint16_t id, const uint8_t *data, uint8_t length)
{
    switch (id) {
        case CAN_ID_GAME_OVER:
            if (screen == SCREEN_GAME && game_state == GAME_PLAYING) {
                player_time[current_player] = oled_ticks() - game_start_tick;
                game_state = GAME_OVER;
            }
            break;

        case CAN_ID_LOG:
            console_write((const char *)data, length);
            break;
    }
}


// ---------------------------------------------------------------- drawing

// ticks -> "m:ss.t"
static void format_time(char *buf, uint8_t size, uint32_t ticks)
{
    uint32_t tenths = ticks * 10 / OLED_TICKS_PER_SECOND;
    snprintf(buf, size, "%u:%02u.%u",
             (unsigned int)(tenths / 600),
             (unsigned int)((tenths / 10) % 60),
             (unsigned int)(tenths % 10));
}

// 0, 1, 2, 1, 0, 1, ... changes about 7 times a second, drives the pulsing outline
static uint8_t pulse(uint32_t now)
{
    static const uint8_t steps[4] = {0, 1, 2, 1};
    return steps[(now / 8) % 4];
}

// button with a label in the middle. when focused it gets a second outline inside, and a
// third outline outside that pulses out and in (needs 2 px free around the button).
// buttons too low for the inner outline are drawn inverted instead.
static void draw_button(int16_t x, int16_t y, int16_t w, int16_t h,
                        const char *label_P, uint8_t scale, uint8_t focused, uint32_t now)
{
    int16_t text_w = GFX_TEXT_WIDTH(strlen_P(label_P), scale);
    int16_t text_h = GFX_CHAR_HEIGHT * scale;
    int16_t text_x = x + (w - text_w) / 2;
    int16_t text_y = y + (h - text_h + 1) / 2;

    gfx_rect(x, y, w, h);

    if (focused && h >= text_h + 8) {
        gfx_rect(x + 2, y + 2, w - 4, h - 4);
        uint8_t out = pulse(now);
        if (out) {
            gfx_rect(x - out, y - out, w + 2 * out, h + 2 * out);
        }
        gfx_text_P(text_x, text_y, label_P, scale, GFX_SET);
    } else if (focused) {
        gfx_fill_rect(x, y, w, h, GFX_SET);
        gfx_text_P(text_x, text_y, label_P, scale, GFX_CLEAR);
    } else {
        gfx_text_P(text_x, text_y, label_P, scale, GFX_SET);
    }
}

// one line list item, inverted when focused
static void draw_list_item(int16_t y, const char *label_P, uint8_t focused)
{
    if (focused) {
        gfx_fill_rect(8, y, 112, 10, GFX_SET);
        gfx_text_P(14, y + 2, label_P, 1, GFX_CLEAR);
    } else {
        gfx_text_P(14, y + 2, label_P, 1, GFX_SET);
    }
}

// inverted bar at the top of a subpage: "< TITLE"
static void draw_header(const char *title_P)
{
    gfx_fill_rect(0, 0, OLED_WIDTH, 9, GFX_SET);
    gfx_text_P(2, 1, PSTR("<"), 1, GFX_CLEAR);
    gfx_text_P(12, 1, title_P, 1, GFX_CLEAR);
}

// small triangle pointing up (dir = 0) or down (dir = 1), 7 px wide, 4 px high
static void draw_arrow(int16_t center_x, int16_t y, uint8_t dir)
{
    for (int16_t i = 0; i < 4; i++) {
        int16_t row = dir ? (y + 3 - i) : (y + i);
        gfx_fill_rect(center_x - i, row, 2 * i + 1, 1, GFX_SET);
    }
}


static void draw_home(uint32_t now)
{
    draw_button(8, 2, 112, 24, PSTR("DEMO"), 2, home_index == HOME_DEMO, now);
    draw_list_item(31, PSTR("Info"),    home_index == HOME_INFO);
    draw_list_item(42, PSTR("Sensors"), home_index == HOME_SENSORS);
    draw_list_item(53, PSTR("Console"), home_index == HOME_CONSOLE);
}

static void draw_demo_setup(uint32_t now)
{
    draw_header(PSTR("DEMO"));

    gfx_text_P(16, 19, PSTR("Players"), 1, GFX_SET);
    char count[2] = {'0' + player_count, '\0'};
    gfx_text(84, 16, count, 2, GFX_SET);
    if (player_count < MAX_PLAYERS) {
        draw_arrow(104, 14, 0);
    }
    if (player_count > 1) {
        draw_arrow(104, 26, 1);
    }

    draw_button(24, 38, 80, 22, PSTR("START"), 1, 1, now);
}

// "PLAYER 2/3" (or just "PLAYER" for one player) centered at y
static void draw_player_title(int16_t y)
{
    char text[12];
    if (player_count > 1) {
        snprintf(text, sizeof(text), "PLAYER %u/%u", current_player + 1, player_count);
    } else {
        snprintf(text, sizeof(text), "PLAYER");
    }
    gfx_text_centered(y, text, 1, GFX_SET);
}

static void draw_exit_hint(void)
{
    gfx_text_centered_P(56, PSTR("hold < to exit"), 1, GFX_SET);
}

static void draw_results(void)
{
    gfx_text_centered_P(1, PSTR("RESULTS"), 1, GFX_SET);

    uint8_t winner = 0;
    for (uint8_t i = 1; i < player_count; i++) {
        if (player_time[i] > player_time[winner]) {
            winner = i;
        }
    }

    for (uint8_t i = 0; i < player_count; i++) {
        int16_t y = 11 + i * 10;
        char text[20];
        char time[10];
        format_time(time, sizeof(time), player_time[i]);
        snprintf(text, sizeof(text), "P%u  %s%s", i + 1, time, (i == winner && player_count > 1) ? "  WIN" : "");

        if (i == winner && player_count > 1) {
            gfx_fill_rect(8, y, 112, 10, GFX_SET);
            gfx_text(14, y + 2, text, 1, GFX_CLEAR);
        } else {
            gfx_text(14, y + 2, text, 1, GFX_SET);
        }
    }
}

// shown while left is held in the game: a ring of 16 dots that fill up clockwise
static void draw_exit_progress(uint32_t now)
{
    // dot positions on a circle with radius 12, starting at the top, going clockwise
    static const int8_t ring[16][2] PROGMEM = {
        {0, -12}, {5, -11}, {8, -8}, {11, -5}, {12, 0}, {11, 5}, {8, 8}, {5, 11},
        {0, 12}, {-5, 11}, {-8, 8}, {-11, 5}, {-12, 0}, {-11, -5}, {-8, -8}, {-5, -11},
    };

    uint32_t held = now - left_down_tick;
    if (held < EXIT_DELAY) {
        return;
    }
    uint8_t filled = (held - EXIT_DELAY) * 16 / EXIT_FILL + 1;

    // replaces the whole game screen while it is shown
    oled_clear();

    for (uint8_t i = 0; i < 16; i++) {
        int16_t x = 64 + (int8_t)pgm_read_byte(&ring[i][0]);
        int16_t y = 26 + (int8_t)pgm_read_byte(&ring[i][1]);
        if (i < filled) {
            gfx_fill_rect(x - 1, y - 1, 3, 3, GFX_SET);
        } else {
            gfx_pixel(x, y, GFX_SET);
        }
    }
    gfx_text_centered_P(44, PSTR("EXIT"), 1, GFX_SET);
    gfx_text_centered_P(55, PSTR("release to cancel"), 1, GFX_SET);
}

static void draw_game(uint32_t now)
{
    char time[10];

    switch (game_state) {
        case GAME_READY:
            draw_player_title(4);
            draw_button(14, 16, 100, 36, PSTR("START"), 2, 1, now);
            draw_exit_hint();
            break;

        case GAME_PLAYING:
            draw_player_title(4);
            format_time(time, sizeof(time), now - game_start_tick);
            gfx_text_centered(22, time, 2, GFX_SET);
            draw_exit_hint();
            break;

        case GAME_OVER: {
            gfx_text_centered_P(2, PSTR("GAME OVER"), 2, GFX_SET);
            char text[20];
            format_time(time, sizeof(time), player_time[current_player]);
            snprintf(text, sizeof(text), "P%u time %s", current_player + 1, time);
            gfx_text_centered(22, text, 1, GFX_SET);
            if (current_player + 1 < player_count) {
                draw_button(14, 36, 100, 24, PSTR("NEXT PLAYER"), 1, 1, now);
            } else {
                draw_button(14, 36, 100, 24, PSTR("RESULTS"), 1, 1, now);
            }
            break;
        }

        case GAME_RESULTS:
            draw_results();
            draw_button(24, 52, 80, 12, PSTR("RESTART"), 1, 1, now);
            break;
    }

    if (left_holding) {
        draw_exit_progress(now);
    }
}

static void draw_info(void)
{
    draw_header(PSTR("INFO"));
    gfx_text_P(4, 16, PSTR("System information"), 1, GFX_SET);
    gfx_text_P(4, 28, PSTR("comes later."), 1, GFX_SET);
}

static void draw_sensors(void)
{
    draw_header(PSTR("SENSORS"));
    // raw adc values for now (joystick x/y, slider ...), real sensor page comes later
    for (uint8_t ch = 0; ch < 4; ch++) {
        char text[12];
        snprintf(text, sizeof(text), "ADC%u  %u", ch, adc_read(ch));
        gfx_text(4, 14 + ch * 10, text, 1, GFX_SET);
    }
}

static void draw_console(void)
{
    draw_header(PSTR("CONSOLE"));
    uint8_t count = console_line_count();
    for (uint8_t i = 0; i < count; i++) {
        gfx_text(0, 10 + i * 8, console_line(i), 1, GFX_SET);
    }
}

void menu_draw(uint32_t now)
{
    oled_clear();

    switch (screen) {
        case SCREEN_HOME:       draw_home(now);       break;
        case SCREEN_DEMO_SETUP: draw_demo_setup(now); break;
        case SCREEN_GAME:       draw_game(now);       break;
        case SCREEN_INFO:       draw_info();          break;
        case SCREEN_SENSORS:    draw_sensors();       break;
        case SCREEN_CONSOLE:    draw_console();       break;
    }
}
