#ifndef CAN_H
#define CAN_H
#include <stdint.h>


typedef enum {
    CAN_FRAME_STANDARD,
    CAN_FRAME_EXTENDED,
    CAN_FRAME_REMOTE,
} CAN_FRAME_TYPE;

typedef enum {
    CAN_STANDARD_ID,
    CAN_EXTENDED_ID
} CAN_ID_TYPE;

typedef struct {
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;

    CAN_ID_TYPE id_format;
    CAN_FRAME_TYPE frame_type;
} CAN_FRAME;


void can_send();
uint16_t can_crc(const uint8_t *data, size_t length);

void can_init();



/*

void can_init(void);
void can_send(CAN_FRAME message);
CAN_FRAME can_recieve(void)
*/
#endif