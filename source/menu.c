#include "../include/menu.h"
#include "../include/gfx.h"
#include "../include/oled.h"
#include "../include/console.h"
#include "../include/settings.h"
#include "../include/can_ids.h"
#include "../include/MCP2515.h"
#include "../include/adc.h"
#include "../include/utils.h"

#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

// Screens:
//   HOME          Demo (big button), then a scrolling list: Info, Sensors, Console, Settings.
//                 up/down to move, click to open
//   DEMO_SETUP    player count (up/down, 1-4) and Start (click). left goes back home
//   GAME          full screen game, one round per player. hold left to exit to DEMO_SETUP
//   INFO/SENSORS  text pages, up/down scrolls. left goes back home
//   CONSOLE       log lines, up/down scrolls back in history. left goes back home
//   SETTINGS      up/down picks a setting, click edits it (up/down changes, click/left done)
//
// Text goes through PSTR/snprintf_P so it stays in flash: the atmega162 only has 1 kB of ram.

typedef enum {
    SCREEN_HOME,
    SCREEN_DEMO_SETUP,
    SCREEN_GAME,
    SCREEN_INFO,
    SCREEN_SENSORS,
    SCREEN_CONSOLE,
    SCREEN_SETTINGS,
} Screen;

typedef enum {
    GAME_READY,     // "player n" and a start button
    GAME_PLAYING,   // timer running until node 2 says the ball is lost
    GAME_OVER,      // time for this player, then next player or results
    GAME_RESULTS,   // all times, winner highlighted, restart
} GameState;

// home options, in the order they are shown
enum { HOME_DEMO, HOME_INFO, HOME_SENSORS, HOME_CONSOLE, HOME_SETTINGS, HOME_COUNT };
#define HOME_LIST_VISIBLE 3 // list items under the demo button that fit on the screen

#define MAX_PLAYERS 4

// exit the game by holding left: nothing happens for EXIT_DELAY, then a circle fills over EXIT_FILL
#define EXIT_DELAY (OLED_TICKS_PER_SECOND / 2)
#define EXIT_FILL  (OLED_TICKS_PER_SECOND * 3 / 2)

// holding up/down repeats the press, for scrolling and changing values
#define REPEAT_DELAY (OLED_TICKS_PER_SECOND * 2 / 5)
#define REPEAT_EVERY (OLED_TICKS_PER_SECOND / 12)

// text pages (info, sensors): rows below the header
#define TEXT_TOP 11
#define TEXT_ROW_H 9
#define TEXT_ROWS_VISIBLE 6
#define INFO_ROWS 7
#define SENSOR_ROWS 12

// console page
#define CONSOLE_TOP 10
#define CONSOLE_VISIBLE 7

// settings page: section headers and settings in one scrolling list
enum { ROW_HEADER_CONTROL, ROW_SCHEME, ROW_HEADER_MAXES, ROW_MAX_SPEED, ROW_HEADER_PID, ROW_P, ROW_I, ROW_D, SETTINGS_ROWS };
#define SETTINGS_TOP 11
#define SETTINGS_ROW_H 10
#define SETTINGS_VISIBLE 5

static Screen screen = SCREEN_HOME;
static uint8_t home_index = HOME_DEMO;
static uint8_t home_list_top = 0;   // first list item shown (0 = Info)
static uint8_t player_count = 1;

static GameState game_state = GAME_READY;
static uint8_t current_player = 0;
static uint32_t game_start_tick = 0;
static uint32_t player_time[MAX_PLAYERS]; // in ticks

static uint8_t previous_nav = 0;    // nav buttons last update, to find new presses
static uint32_t repeat_start = 0;   // when up/down was pressed
static uint32_t last_repeat = 0;
static uint8_t left_holding = 0;    // left is being held in the game
static uint32_t left_down_tick = 0;

static uint8_t page_top = 0;        // scroll position on info and sensors
static uint8_t console_scroll = 0;  // lines scrolled back from the newest
static uint16_t console_seen = 0;   // console_lines_added() when we last looked

static uint8_t settings_row = ROW_SCHEME;
static uint8_t settings_top = 0;
static uint8_t settings_editing = 0;

static Joystick input_joystick;
static Slider input_slider;
static Touchpad input_touchpad;
static uint8_t can_ok = 1;


// ---------------------------------------------------------------- helpers

// move `top` so that `selected` is inside the visible window
static void scroll_to(uint8_t *top, uint8_t selected, uint8_t visible)
{
    if (selected < *top) {
        *top = selected;
    }
    if (selected >= *top + visible) {
        *top = selected - visible + 1;
    }
}

static void send_can(uint16_t id)
{
    uint8_t nothing = 0;
    mcp_send_can_message(id, &nothing, 0);
}

void menu_set_inputs(Joystick joystick, Slider slider, Touchpad touchpad)
{
    input_joystick = joystick;
    input_slider = slider;
    input_touchpad = touchpad;
}

void menu_set_can_ok(uint8_t ok)
{
    can_ok = ok;
}


// ---------------------------------------------------------------- game

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


// ---------------------------------------------------------------- settings

static uint8_t is_settings_header(uint8_t row)
{
    return row == ROW_HEADER_CONTROL || row == ROW_HEADER_MAXES || row == ROW_HEADER_PID;
}

// step a value up (dir = 1) or down (dir = -1) inside its range
static void settings_adjust(uint8_t row, int8_t dir)
{
    uint16_t *value;
    int16_t min, max, step;
    switch (row) {
        case ROW_MAX_SPEED: value = &settings.max_speed; min = 500; max = 4200; step = 100; break;
        case ROW_P:         value = &settings.pid[0];    min = 0;   max = 2000; step = 25;  break; // 0-20.00
        case ROW_I:         value = &settings.pid[1];    min = 0;   max = 1000; step = 10;  break; // 0-10.00
        case ROW_D:         value = &settings.pid[2];    min = 0;   max = 200;  step = 5;   break; // 0-2.00
        default: return;
    }
    int16_t next = (int16_t)*value + dir * step;
    if (next < min) next = min;
    if (next > max) next = max;
    *value = next;
}

static void settings_update(Buttons pressed)
{
    if (settings_editing) {
        if (pressed.NU) {
            settings_adjust(settings_row, 1);
        }
        if (pressed.ND) {
            settings_adjust(settings_row, -1);
        }
        if (pressed.NB || pressed.NL) {
            settings_editing = 0;
            settings_send();
        }
        return;
    }

    // move between settings, skipping the section headers
    if (pressed.NU) {
        for (int8_t row = settings_row - 1; row >= 0; row--) {
            if (!is_settings_header(row)) {
                settings_row = row;
                break;
            }
        }
    }
    if (pressed.ND) {
        for (uint8_t row = settings_row + 1; row < SETTINGS_ROWS; row++) {
            if (!is_settings_header(row)) {
                settings_row = row;
                break;
            }
        }
    }
    scroll_to(&settings_top, settings_row, SETTINGS_VISIBLE);
    // also show the section header right above the selected setting
    if (settings_row > 0 && is_settings_header(settings_row - 1) && settings_top == settings_row) {
        settings_top--;
    }

    if (pressed.NB) {
        if (settings_row == ROW_SCHEME) {
            settings.scheme = (settings.scheme == CONTROL_POSITION) ? CONTROL_VELOCITY : CONTROL_POSITION;
            settings_send();
        } else {
            settings_editing = 1;
        }
    }
    if (pressed.NL) {
        screen = SCREEN_HOME;
    }
}


// ---------------------------------------------------------------- input

static void open_home_item(void)
{
    page_top = 0;
    switch (home_index) {
        case HOME_DEMO:     screen = SCREEN_DEMO_SETUP; break;
        case HOME_INFO:     screen = SCREEN_INFO;       break;
        case HOME_SENSORS:  screen = SCREEN_SENSORS;    break;
        case HOME_CONSOLE:
            screen = SCREEN_CONSOLE;
            console_scroll = 0; // start at the newest line
            console_seen = console_lines_added();
            break;
        case HOME_SETTINGS:
            screen = SCREEN_SETTINGS;
            settings_editing = 0;
            break;
    }
}

// up/down scroll on a page with `rows` rows of which `visible` fit
static void scroll_page(Buttons pressed, uint8_t rows, uint8_t visible)
{
    if (pressed.NU && page_top > 0) {
        page_top--;
    }
    if (pressed.ND && page_top + visible < rows) {
        page_top++;
    }
}

static void console_update(Buttons pressed)
{
    uint8_t count = console_line_count();
    uint8_t max_scroll = count > CONSOLE_VISIBLE ? count - CONSOLE_VISIBLE : 0;

    // when scrolled back, keep looking at the same lines while new ones arrive
    uint16_t added = console_lines_added();
    if (console_scroll > 0) {
        console_scroll += added - console_seen;
    }
    console_seen = added;

    if (pressed.NU) {
        console_scroll++;
    }
    if (pressed.ND && console_scroll > 0) {
        console_scroll--;
    }
    if (console_scroll > max_scroll) {
        console_scroll = max_scroll;
    }
}

void menu_update(Buttons buttons, uint32_t now)
{
    // only react when a button goes down, not every update while it is held
    Buttons pressed = {0};
    pressed.nav = buttons.nav & ~previous_nav;
    previous_nav = buttons.nav;

    // ...except up/down, which repeat while held
    if (pressed.NU || pressed.ND) {
        repeat_start = now;
        last_repeat = now;
    } else if ((buttons.NU || buttons.ND) &&
               now - repeat_start >= REPEAT_DELAY && now - last_repeat >= REPEAT_EVERY) {
        pressed.NU = buttons.NU;
        pressed.ND = buttons.ND;
        last_repeat = now;
    }

    switch (screen) {
        case SCREEN_HOME:
            if (pressed.NU && home_index > 0) {
                home_index--;
            }
            if (pressed.ND && home_index < HOME_COUNT - 1) {
                home_index++;
            }
            if (home_index == HOME_DEMO) {
                home_list_top = 0;
            } else {
                scroll_to(&home_list_top, home_index - 1, HOME_LIST_VISIBLE);
            }
            if (pressed.NB) {
                open_home_item();
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
            scroll_page(pressed, INFO_ROWS, TEXT_ROWS_VISIBLE);
            if (pressed.NL) {
                screen = SCREEN_HOME;
            }
            break;

        case SCREEN_SENSORS:
            scroll_page(pressed, SENSOR_ROWS, TEXT_ROWS_VISIBLE);
            if (pressed.NL) {
                screen = SCREEN_HOME;
            }
            break;

        case SCREEN_CONSOLE:
            console_update(pressed);
            if (pressed.NL) {
                screen = SCREEN_HOME;
            }
            break;

        case SCREEN_SETTINGS:
            settings_update(pressed);
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
    snprintf_P(buf, size, PSTR("%u:%02u.%u"),
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

// dotted track with a solid thumb, only drawn when there is something to scroll
static void draw_scrollbar(int16_t x, int16_t y, int16_t h, uint8_t top, uint8_t visible, uint8_t total)
{
    if (total <= visible) {
        return;
    }
    for (int16_t i = y; i < y + h; i += 2) {
        gfx_pixel(x, i, GFX_SET);
    }
    int16_t thumb = (int16_t)((int32_t)h * visible / total);
    if (thumb < 4) {
        thumb = 4;
    }
    int16_t pos = y + (int16_t)((int32_t)(h - thumb) * top / (total - visible));
    gfx_fill_rect(x - 1, pos, 3, thumb, GFX_SET);
}

// small triangle pointing up (dir = 0) or down (dir = 1), 7 px wide, 4 px high
static void draw_arrow(int16_t center_x, int16_t y, uint8_t dir)
{
    for (int16_t i = 0; i < 4; i++) {
        int16_t row = dir ? (y + 3 - i) : (y + i);
        gfx_fill_rect(center_x - i, row, 2 * i + 1, 1, GFX_SET);
    }
}


static const char *home_label(uint8_t index)
{
    switch (index) {
        case HOME_INFO:     return PSTR("Info");
        case HOME_SENSORS:  return PSTR("Sensors");
        case HOME_CONSOLE:  return PSTR("Console");
        default:            return PSTR("Settings");
    }
}

static void draw_home(uint32_t now)
{
    draw_button(8, 2, 112, 24, PSTR("DEMO"), 2, home_index == HOME_DEMO, now);

    for (uint8_t i = 0; i < HOME_LIST_VISIBLE; i++) {
        uint8_t index = home_list_top + i + 1; // + 1: the list starts after demo
        draw_list_item(31 + i * 11, home_label(index), home_index == index);
    }
    draw_scrollbar(124, 31, 32, home_list_top, HOME_LIST_VISIBLE, HOME_COUNT - 1);
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
        snprintf_P(text, sizeof(text), PSTR("PLAYER %u/%u"), current_player + 1, player_count);
        gfx_text_centered(y, text, 1, GFX_SET);
    } else {
        gfx_text_centered_P(y, PSTR("PLAYER"), 1, GFX_SET);
    }
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
        uint8_t is_winner = (i == winner && player_count > 1);
        char text[20];
        char time[10];
        format_time(time, sizeof(time), player_time[i]);
        snprintf_P(text, sizeof(text), is_winner ? PSTR("P%u  %s  WIN") : PSTR("P%u  %s"), i + 1, time);

        if (is_winner) {
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
            snprintf_P(text, sizeof(text), PSTR("P%u time %s"), current_player + 1, time);
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

// one row of the info page (max 20 characters)
static void info_row(uint8_t row, char *buf, uint8_t size)
{
    uint32_t seconds = oled_ticks() / OLED_TICKS_PER_SECOND;
    switch (row) {
        case 0: strcpy_P(buf, PSTR("Node 1  ATmega162")); break;
        case 1: strcpy_P(buf, PSTR("Node 2  Arduino Due")); break;
        case 2: strcpy_P(buf, PSTR("CAN     500 kbit/s")); break;
        case 3: strcpy_P(buf, can_ok ? PSTR("Node 2  answering") : PSTR("Node 2  no answer")); break;
        case 4:
            snprintf_P(buf, size, PSTR("Uptime  %u:%02u:%02u"),
                       (unsigned int)(seconds / 3600),
                       (unsigned int)((seconds / 60) % 60),
                       (unsigned int)(seconds % 60));
            break;
        case 5: snprintf_P(buf, size, PSTR("Free RAM %u bytes"), utils_free_ram()); break;
        default: strcpy_P(buf, PSTR("Built   " __DATE__)); break;
    }
}

// one row of the sensors page (max 20 characters)
static void sensor_row(uint8_t row, char *buf, uint8_t size)
{
    switch (row) {
        case 0: snprintf_P(buf, size, PSTR("Joystick X  %u"), input_joystick.x); break;
        case 1: snprintf_P(buf, size, PSTR("Joystick Y  %u"), input_joystick.y); break;
        case 2: snprintf_P(buf, size, PSTR("Joy button  %u"), input_joystick.btn); break;
        case 3: snprintf_P(buf, size, PSTR("Slider      %u"), input_slider.x); break;
        case 4: snprintf_P(buf, size, PSTR("Slider size %u"), input_slider.size); break;
        case 5: snprintf_P(buf, size, PSTR("Touch X     %u"), input_touchpad.x); break;
        case 6: snprintf_P(buf, size, PSTR("Touch Y     %u"), input_touchpad.y); break;
        case 7: snprintf_P(buf, size, PSTR("Touch size  %u"), input_touchpad.size); break;
        default: snprintf_P(buf, size, PSTR("ADC%u        %u"), row - 8, adc_read(row - 8)); break;
    }
}

// header and rows from `row_text`, scrolled to page_top
static void draw_text_page(const char *title_P, void (*row_text)(uint8_t, char *, uint8_t), uint8_t rows)
{
    draw_header(title_P);
    for (uint8_t i = 0; i < TEXT_ROWS_VISIBLE && page_top + i < rows; i++) {
        char text[22];
        row_text(page_top + i, text, sizeof(text));
        gfx_text(2, TEXT_TOP + i * TEXT_ROW_H, text, 1, GFX_SET);
    }
    draw_scrollbar(125, 11, 53, page_top, TEXT_ROWS_VISIBLE, rows);
}

static void draw_console(void)
{
    draw_header(PSTR("CONSOLE"));
    uint8_t count = console_line_count();
    uint8_t first = 0;
    if (count > CONSOLE_VISIBLE) {
        first = count - CONSOLE_VISIBLE - console_scroll;
    }
    for (uint8_t i = 0; i < CONSOLE_VISIBLE && first + i < count; i++) {
        gfx_text(0, CONSOLE_TOP + i * 8, console_line(first + i), 1, GFX_SET);
    }
    draw_scrollbar(126, 11, 53, first, CONSOLE_VISIBLE, count);
}

// value of a setting as text
static void settings_value(uint8_t row, char *buf, uint8_t size)
{
    switch (row) {
        case ROW_SCHEME:
            strcpy_P(buf, settings.scheme == CONTROL_VELOCITY ? PSTR("Velocity") : PSTR("Position"));
            break;
        case ROW_MAX_SPEED:
            snprintf_P(buf, size, PSTR("%u"), settings.max_speed);
            break;
        default: {
            uint16_t v = settings.pid[row - ROW_P];
            snprintf_P(buf, size, PSTR("%u.%02u"), v / 100, v % 100);
            break;
        }
    }
}

static const char *settings_label(uint8_t row)
{
    switch (row) {
        case ROW_HEADER_CONTROL: return PSTR("Control scheme");
        case ROW_SCHEME:         return PSTR("Mode");
        case ROW_HEADER_MAXES:   return PSTR("Maxes");
        case ROW_MAX_SPEED:      return PSTR("Max speed");
        case ROW_HEADER_PID:     return PSTR("PID");
        case ROW_P:              return PSTR("P");
        case ROW_I:              return PSTR("I");
        default:                 return PSTR("D");
    }
}

static void draw_settings(void)
{
    draw_header(PSTR("SETTINGS"));

    for (uint8_t i = 0; i < SETTINGS_VISIBLE && settings_top + i < SETTINGS_ROWS; i++) {
        uint8_t row = settings_top + i;
        int16_t y = SETTINGS_TOP + i * SETTINGS_ROW_H;
        const char *label = settings_label(row);

        if (is_settings_header(row)) {
            // section title with a line after it
            gfx_text_P(2, y + 2, label, 1, GFX_SET);
            int16_t line_x = 2 + GFX_TEXT_WIDTH(strlen_P(label), 1) + 4;
            gfx_fill_rect(line_x, y + 5, 120 - line_x, 1, GFX_SET);
            continue;
        }

        char value[10];
        settings_value(row, value, sizeof(value));
        int16_t value_w = GFX_TEXT_WIDTH(strlen(value), 1);
        int16_t value_x = 117 - value_w;

        if (row == settings_row && settings_editing) {
            // editing: only the value is inverted, up/down change it
            gfx_text_P(10, y + 2, label, 1, GFX_SET);
            gfx_fill_rect(value_x - 3, y, value_w + 6, 10, GFX_SET);
            gfx_text(value_x, y + 2, value, 1, GFX_CLEAR);
        } else if (row == settings_row) {
            gfx_fill_rect(4, y, 116, 10, GFX_SET);
            gfx_text_P(10, y + 2, label, 1, GFX_CLEAR);
            gfx_text(value_x, y + 2, value, 1, GFX_CLEAR);
        } else {
            gfx_text_P(10, y + 2, label, 1, GFX_SET);
            gfx_text(value_x, y + 2, value, 1, GFX_SET);
        }
    }
    draw_scrollbar(124, 11, 53, settings_top, SETTINGS_VISIBLE, SETTINGS_ROWS);
}

void menu_draw(uint32_t now)
{
    oled_clear();

    switch (screen) {
        case SCREEN_HOME:       draw_home(now);       break;
        case SCREEN_DEMO_SETUP: draw_demo_setup(now); break;
        case SCREEN_GAME:       draw_game(now);       break;
        case SCREEN_INFO:       draw_text_page(PSTR("INFO"), info_row, INFO_ROWS);         break;
        case SCREEN_SENSORS:    draw_text_page(PSTR("SENSORS"), sensor_row, SENSOR_ROWS);  break;
        case SCREEN_CONSOLE:    draw_console();       break;
        case SCREEN_SETTINGS:   draw_settings();      break;
    }
}
