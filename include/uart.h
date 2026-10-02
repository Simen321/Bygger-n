#ifndef UART_H
#define UART_H

void uart1_init(void);
int uart_sendltr(char c, FILE *stream);
int uart_getltr(FILE *stream);
void cleanArray(unsigned char arr[10]);


#endif