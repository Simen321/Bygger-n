#ifndef MCP2515_H
#define MCP2515_H

#include <stdint.h>



uint8_t MCP2515_write_read(uint8_t dataIn);
void MCP2515_write(uint8_t address, uint8_t data);
void MCP2515_req_to_send(uint8_t tx_buffers);
uint8_t MCP2515_read_status(void);
void MCP2515_bit_modify(uint8_t address, uint8_t mask, uint8_t data);
void MCP2515_reset (void);
uint8_t MCP2515_write(uint8_t);
uint8_t MCP2515_read(void);

void MCP2515_print_all(void);
void lets_test_the_fucking_MCP2515(void);

#endif