#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

// log text history, shown (and scrolled) on the console page.
// text comes from node 2 over CAN (CAN_ID_LOG) and from node 1 itself.
//
// the lines are kept in external sram, right after the oled framebuffer (0x1000-0x13FF),
// because 40 lines would use almost all of the atmega162's 1 kB internal ram.

#define CONSOLE_HISTORY 40               // lines kept
#define CONSOLE_LINE_LEN 21              // characters per line (128 px / 6 px)
#ifndef CONSOLE_BUFFER_ADDR
#define CONSOLE_BUFFER_ADDR 0x1400       // 40 * 22 bytes = 0x1400-0x1770
#endif

void console_init(void);                           // call after the external sram is enabled
void console_write(const char *data, uint8_t len); // raw text, '\n' starts a new line
void console_log_P(const char *str);               // a whole line from flash (PSTR), '\n' added

uint8_t console_line_count(void);         // lines in use, at most CONSOLE_HISTORY
const char *console_line(uint8_t index);  // 0 is the oldest line, the last one is the newest
uint16_t console_lines_added(void);       // counts every new line, so the page can tell when text arrived

#endif
