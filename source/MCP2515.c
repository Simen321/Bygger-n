#include "../include/MCP2515.h"
#include "../include/spi.h"
void mcp_reset(void) {
    spi_select_slave_n(3); // 3 is hardcoded for MCP2515 
    spi_write_byte(RESET);
    spi_select_slave_n(0);
}

void mcp_init(MODES_OF_OPERATION mode){
    
    uint8_t data3 = mcp_read(0x0F);

    


    printf("got back read: %d \r\n", data3);
    printf("got back read: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (data3 >> i) & 1);
    }
    printf("\r\n");

    mcp_reset();
    // CNF1, CNF2, CNF3
    // TXRTSCTRL
    // Filter registers
    // Mask registers
    printf("Mode value: %d\r\n", mode);
    mcp_bit_modify(0x0F, 0xE0, mode); // Sets it into selected mode of operation
    

    
    
    uint8_t data2 = mcp_read(0x0F);

    


    printf("got back read: %d \r\n", data2);
    printf("got back read: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (data2 >> i) & 1);
    }
    printf("\r\n");



}

uint8_t mcp_read(uint8_t address) {
    spi_select_slave_n(3);
    spi_write_byte(READ);
    spi_write_byte(address); // read spesific register address
    uint8_t data = spi_read_byte();
    spi_select_slave_n(0);
    return data;
}

uint8_t mcp_read_rx_buffer(uint8_t n, uint8_t m) {
    spi_select_slave_n(3);
    uint8_t cmd = READ_RX_BUFFER | (n << 2) | (m << 1); // sets the 1001 0nm0 cmd depending on inpt
    spi_write_byte(cmd);
    uint8_t data = spi_read_byte();
    spi_select_slave_n(0);
    return data;
}

void mcp_write(uint8_t address, uint8_t data){
    spi_select_slave_n(3); // 3 is hardcoded for MCP2515 
    spi_write_byte(WRITE);
    spi_write_byte(address);
    spi_write_byte(data);
    spi_select_slave_n(0);
}

void mcp_load_tx_buffer(uint8_t a, uint8_t b, uint8_t c, uint8_t data){
    spi_select_slave_n(3);
    uint8_t cmd = LOAD_TX_BUFFER | (a << 2) | (b << 1) | (c << 0);
    spi_write_byte(cmd);
    spi_write_byte(data);
    spi_select_slave_n(0);
}

void mcp_rts(uint8_t n){
    spi_select_slave_n(3);
    uint8_t cmd = RTS | (n & 0x07); // her blir da inputtn 1 11 eller 111.
    spi_write_byte(cmd);
    spi_select_slave_n(0);
}

uint8_t mcp_read_status(void) {
    spi_select_slave_n(3);
    spi_write_byte(READ_STATUS);
    uint8_t data = spi_read_byte();
    spi_select_slave_n(0);
    return data;
}

uint8_t mcp_read_rx_status(void){
    spi_select_slave_n(3);
    spi_write_byte(RX_STATUS);
    uint8_t data = spi_read_byte();
    spi_select_slave_n(0);
    return data;
}

void mcp_bit_modify(uint8_t address, uint8_t mask_byte, uint8_t data_byte){
    spi_select_slave_n(3);
    spi_write_byte(BIT_MODIFY);
    spi_write_byte(address);
    spi_write_byte(mask_byte);
    spi_write_byte(data_byte);
    spi_select_slave_n(0);
}


