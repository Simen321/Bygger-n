#include "../include/can.h"
#include <stddef.h>
#include "../include/MCP2515.h"
/*
void can_init(void) {

}
*/

#define BRP 0 //baud rate prescaler set to 0 for maximum speed
#define CRYSTAL 16000000UL // clock frequency on the crystal
void can_init() {
    // our clock speed that is fed from the new crystal, at 16Mhz
    // CNF1, CNF2, CNF3

    mcp_bit_modify(0xA2, 255, 0b00000000); // setter CNF1 registeret lik 0 på alle 
    // bits siden vi bruker 16Mhz klokkefrekvens og vi ikke trenger noe prescaler, dette setter det til BRP+1 = 1 her.
    

    // TXRTSCTRL
    // Filter registers
    // Mask registers
    // we need to set its baudrate etc.



}


void can_send() {
    
    uint8_t id = 1;

    uint8_t length = 8;
    uint8_t payload[8] = {67, 67, 67, 67, 67, 67, 67, 67};



    uint8_t CRC = can_crc(payload, length);
}

uint16_t can_crc(const uint8_t *data, size_t length) {
    //uint8_t allone = 0b11111111; // all positive
    const uint16_t POLYNOMIAL = 0x4599; 
    uint16_t crc = 0x0000;
    
    for (uint8_t i = 0; i < length; i++) {

        for (uint8_t j = 7; j >= 0; j--) {
            uint8_t bit = (data[i] >> j) & 1;
            uint8_t msb = (crc >> 14) & 1;
            crc = ((crc << 1) | bit) & 0x7FFF;
            
            if (msb ^ bit) {
                crc ^= POLYNOMIAL;
            }
        }
    }
    return crc;
}
/*
CAN_STANDARD_FRAME can_recieve(void){

}*/