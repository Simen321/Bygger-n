#ifndef F_CPU
#define F_CPU 4915200UL
#endif

#include "../include/MCP2515.h"
#include "../include/spi.h"
#include <util/delay.h>

void mcp_reset(void) {
    spi_select_slave_n(3); // 3 is hardcoded for MCP2515 
    spi_write_byte(RESET);
    spi_select_slave_n(0);
}

void mcp_init(MODES_OF_OPERATION mode){
    
    uint8_t data3 = mcp_read(0x0F);

    


    //printf("got back read: %d \r\n", data3);
    //printf("got back read: ");
    for (int i = 7; i >= 0; i--) {
        //printf("%d", (data3 >> i) & 1);
    }
    //printf("\r\n");

    mcp_reset();
    _delay_ms(1); // the mcp2515 needs a moment after reset before it accepts spi commands (oscillator start-up)
        // our clock speed that is fed from the new crystal, at 16Mhz
    // CNF1, CNF2, CNF3

    //mcp_bit_modify(0x2A, 255, 0b00000000);
    /*
    CNF1 ^^^
    0: JSW1  
    0: JSW0     
    0: BRP5  
    0: BRP4  
    0: BRP3  
    0: BRP2
    0: BRP1  
    0: BRP0
    */

    //mcp_bit_modify(0x29, 255, 0b10010010);
    /*
    CNF2 ^^^
    1: BLTMODE  - styrer phase 2 manuelt
    0: SAM      - sample 1 gang
    0: PHSEG12  
    1: PHSEG11  - 010:2 3Tq
    0: PHSEG10  
    0: PRSEG2
    1: PRSEG1   - 010:2 3Tq
    0: PRSEG0
    */

    //mcp_bit_modify(0x28, 255, 0b00000011);
    /*
    CNF3 ^^^
    0: SOF  
    0: WAKFIL    
    0: -  
    0: - 
    0: -  
    0: PHSEG22
    1: PHSEG21  - 011:3 4Tq
    1: PHSEG20
    */
mcp_bit_modify(0x2A, 0xFF, 0x00);
mcp_bit_modify(0x29, 0xFF, 0xAC);
mcp_bit_modify(0x28, 0xFF, 0x03);
    // ^Dette setter opp slik at mcp2515 leser av bussen etter 7 av 16Tq, som er ish 3/4 av bit-perioden


    
    // TXRTSCTRL(0D): Transmit requrest to send control register

    // receive filters: node 2 sends on 0x200-0x2FF (see can_ids.h), everything else is ignored.
    // mask 0x700 = only the top 3 id bits must match, filter 0x200.
    //   mask/filter SIDH = id >> 3, SIDL = (id << 5) & 0xE0 (bit 3 EXIDE = 0, standard ids only)
    // both receive buffers get the same mask and filters. BUKT (rollover) on RXB0 means a second
    // frame goes to RXB1 when RXB0 is still full, so we can hold two frames between reads.
    mcp_write(0x60, 0x04); // RXB0CTRL: RXM = 00 (use filters), BUKT = 1
    mcp_write(0x70, 0x00); // RXB1CTRL: RXM = 00 (use filters)

    // masks: RXM0 (RXB0) and RXM1 (RXB1)
    mcp_write(0x20, 0xE0); mcp_write(0x21, 0x00);
    mcp_write(0x24, 0xE0); mcp_write(0x25, 0x00);

    // filters: RXF0-1 belong to RXB0, RXF2-5 to RXB1
    const uint8_t filter_sidh_addr[6] = {0x00, 0x04, 0x08, 0x10, 0x14, 0x18};
    for (uint8_t i = 0; i < 6; i++) {
        mcp_write(filter_sidh_addr[i], 0x40);     // RXFnSIDH: 0x200 >> 3
        mcp_write(filter_sidh_addr[i] + 1, 0x00); // RXFnSIDL
    }



    //printf("Mode value: %d\r\n", mode);
    mcp_bit_modify(0x0F, 0xE0, mode); // Sets it into selected mode of operation
    
    //printf("got back read: ");

    
    
    uint8_t data2 = mcp_read(0x0F);

    
    uint8_t actual_mode = 0;
    uint16_t timeout = 0;
    do {
        actual_mode = mcp_read(0x0E) & 0xE0; // Les CANSTAT bit 7,6,5
        timeout++;
        if (timeout > 5000) {
            //printf("FEIL: Modusskifte tok for lang tid! Står fast i: 0x%X\r\n", actual_mode);
            break;
        }
    } while (actual_mode != mode);
    //printf("Brikken bekreftet modus! CANSTAT: 0x%X\r\n", mcp_read(0x0E));
    //printf("got back read: %d \r\n", data2);
    //printf("got back read: ");
    for (int i = 7; i >= 0; i--) {
        //printf("%d", (data2 >> i) & 1);
    }
    //printf("\r\n");



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
void mcp_load_tx_buffer_data(const uint8_t *data, uint8_t length)
{
    spi_select_slave_n(3);

    // LOAD TX BUFFER 0, start ved D0
    spi_write_byte(LOAD_TX_BUFFER | 0x01);

    for (uint8_t i = 0; i < length; i++) {
        spi_write_byte(data[i]);
    }

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


// how many times we check if the previous frame is sent before giving up (each check is ~20 us)
#define MCP_TX_TIMEOUT 100

bool mcp_send_can_message(uint16_t id, const uint8_t *data, uint8_t length) {
    // 1. Vent til TX-Buffer 0 er ledig (TXREQ-biten i TXB0CTRL må være 0)
    // if nobody acks the frame (node 2 off or unplugged) the mcp2515 retries forever,
    // so give up after a while instead of freezing the menu.
    uint8_t timeout = MCP_TX_TIMEOUT;
    while (mcp_read(0x30) & 0x08) {
        if (--timeout == 0) {
            mcp_bit_modify(0x30, 0x08, 0x00); // clear TXREQ: abort the stuck frame
            return false;
        }
    }

    // 2. Formater 11-bit ID inn i brikkens registre
    uint8_t sidh = (id >> 3) & 0xFF;  
    uint8_t sidl = (id << 5) & 0xE0;  

    mcp_write(0x31, sidh); // TXB0SIDH
    mcp_write(0x32, sidl); // TXB0SIDL

    // 3. Sett datalengde (DLC)
    mcp_write(0x35, length & 0x0F); // TXB0DLC

    // 4. LAST INN DATA
    // starter på TXB0D0.
    mcp_load_tx_buffer_data(data, length);

    // 5. TRIGGER SENDINGEN
    // mcp_rts(1) sender kommandoen for TX-Buffer 0
    mcp_rts(1);

    return true;
}

// reads one frame from RXB0 or RXB1. returns false if both are empty.
// call it in a loop to empty both buffers.
bool mcp_receive_can_message(
    uint16_t *out_id,
    uint8_t *out_data,
    uint8_t *out_length)
{
    // CANINTF ligger på 0x2C. bit 0 = RXB0 full, bit 1 = RXB1 full
    uint8_t interrupt_flags = mcp_read(0x2C);

    uint8_t base;   // RXBnCTRL address, the other registers follow it
    uint8_t flag;
    if (interrupt_flags & 0x01) {
        base = 0x60; // RXB0
        flag = 0x01;
    } else if (interrupt_flags & 0x02) {
        base = 0x70; // RXB1
        flag = 0x02;
    } else {
        return false;
    }

    // Les standard CAN-ID
    uint8_t sidh = mcp_read(base + 1);
    uint8_t sidl = mcp_read(base + 2);

    *out_id = ((uint16_t)sidh << 3) | (sidl >> 5);

    // Les DLC
    uint8_t dlc = mcp_read(base + 5);
    *out_length = dlc & 0x0F;

    // Beskytt mot ugyldig DLC
    if (*out_length > 8) {
        *out_length = 8;
    }

    // Les data
    for (uint8_t i = 0; i < *out_length; i++) {
        out_data[i] = mcp_read(base + 6 + i);
    }

    // Nullstill RXnIF så bufferet kan ta imot en ny melding
    mcp_bit_modify(0x2C, flag, 0x00);

    return true;
}
