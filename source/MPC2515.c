#define F_CPU 4915200UL
#define TXB0 0x01
#define TXB1 0x02
#define TXB2 0x04


#include "../include/MCP2515.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>


uint8_t MCP2515_write_read(uint8_t dataIn){
    uint8_t data;
    spi_select_slave(address);
    data = spi_write_read(dataIn);
    spi_select_slave(0);
    return data;
}

uint8_t MCP2515_read(void) {
    spi_select_slave(3);
    uint8_t data = spi_write_read(0x03);
    spi_select_slave(0);
    return data;
}

void MCP2515_write(uint8_t address) {
    spi_select_slave(3);
    uint8_t data = spi_write_read(0x02);
    spi_select_slave(0);
    return data;
}



void MCP2515_req_to_send(uint8_t tx_buffers){
    spi_select_slave(3);
    uint8_t data = spi_write_read(0x80 | TXB0); //(TXB1=0x82 and TXB2=0x84)
    spi_select_slave(0);
}

uint8_t MCP2515_read_status(void){
    spi_select_slave(3);
    uint8_t data;

    return data;
}

void MCP2515_bit_modify(uint8_t address, uint8_t mask, uint8_t data){

}

void MCP2515_reset (void){

}

void MCP2515_print_all(void)
{
    // TODO: print relevante registre

}


void lets_test_the_fucking_MCP2515(void)
{
    // TODO: tester MCP2515

}