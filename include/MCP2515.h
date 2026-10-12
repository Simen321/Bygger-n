#ifndef MCP2515_H
#define MCP2515_H

#include <stdint.h>
#include <stdio.h>

/*
read
write
request to send
read status
bit modify


we need to reset CS every time a new command is to be interperated by the mcp2515


*/

typedef enum {
    RESET = 0xC0,
    READ = 0x03,
    READ_RX_BUFFER = 0x90, // 1001 0nm0
    WRITE = 0x02,
    LOAD_TX_BUFFER = 0x40,
    RTS = 0x80, // 10000nnn
    READ_STATUS = 0xA0,
    RX_STATUS = 0xB0,
    BIT_MODIFY = 0x05,
} MCP2515;
// uint8_t  readrxbuffer= MPC_2515.READ_RX_BUFFER | (n << 2) | (m << 1); for eksample




typedef enum {
    CONFIG_MODE = 0b1000,
    NORMAL_MODE = 0b0000,
    SLEEP_MODE = 0b0010,
    LISTEN_ONLY_MODE = 0b0110,
    LOOPBACK_MODE = 0x40
} MODES_OF_OPERATION;

void mcp_init(MODES_OF_OPERATION mode);
void mcp_reset(void); // low CS -> instruction byte -> high CS
uint8_t mcp_read(uint8_t address);
uint8_t mcp_read_rx_buffer(uint8_t n, uint8_t m);

void mcp_write(uint8_t address, uint8_t data);
void mcp_load_tx_buffer(uint8_t a, uint8_t b, uint8_t c, uint8_t data);

void mcp_rts(uint8_t n);

uint8_t mcp_read_status(void);

uint8_t mcp_read_rx_status(void);

void mcp_bit_modify(uint8_t address, uint8_t mask_byte, uint8_t data_byte);

#endif