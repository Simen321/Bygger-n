#include "../include/console.h"

#include <avr/pgmspace.h>

// ring of lines. `newest` is the line currently being written to.
static char lines[CONSOLE_LINES][CONSOLE_LINE_LEN + 1];
static uint8_t newest = 0;
static uint8_t used = 1;     // lines with content (the newest one counts even while empty)
static uint8_t length = 0;   // characters in the newest line


static void new_line(void)
{
    newest = (newest + 1) % CONSOLE_LINES;
    lines[newest][0] = '\0';
    length = 0;
    if (used < CONSOLE_LINES) {
        used++;
    }
}


static void put_char(char c)
{
    if (c == '\r') {
        return;
    }
    if (c == '\n') {
        new_line();
        return;
    }
    if (length == CONSOLE_LINE_LEN) {
        new_line(); // wrap long lines
    }
    if (c < 32 || c > 126) {
        c = '?';
    }
    lines[newest][length++] = c;
    lines[newest][length] = '\0';
}


void console_write(const char *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        put_char(data[i]);
    }
}


void console_log_P(const char *str)
{
    // finish a half written line from node 2 first, so the two do not get mixed
    if (length > 0) {
        new_line();
    }
    char c;
    while ((c = pgm_read_byte(str++))) {
        put_char(c);
    }
    put_char('\n');
}


uint8_t console_line_count(void)
{
    // a trailing empty line (right after a '\n') is not worth showing
    return (length == 0 && used > 1) ? used - 1 : used;
}


const char *console_line(uint8_t index)
{
    uint8_t first = (newest + CONSOLE_LINES - (used - 1)) % CONSOLE_LINES;
    return lines[(first + index) % CONSOLE_LINES];
}
