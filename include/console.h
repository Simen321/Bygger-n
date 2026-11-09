#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

// the last few lines of log text, shown on the console page.
// text comes from node 2 over CAN (CAN_ID_LOG) and from node 1 itself.

#define CONSOLE_LINES 7      // lines that fit under the page header
#define CONSOLE_LINE_LEN 21  // characters per line (128 px / 6 px)

void console_write(const char *data, uint8_t len); // raw text, '\n' starts a new line
void console_log_P(const char *str);               // a whole line from flash (PSTR), '\n' added

uint8_t console_line_count(void);         // lines in use, at most CONSOLE_LINES
const char *console_line(uint8_t index);  // 0 is the oldest line, the last one is the newest

#endif
