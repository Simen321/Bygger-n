#ifndef UART_H
#define UART_H

#define F_CPU 4915200UL
#define BAUD 9600UL
#define UBRR_VAL ((F_CPU) / (16 * BAUD) - 1) // teh baudrate formula

#include <stdio.h>
#include <stdlib.h>
void uart1_init(void);
int uart_sendltr(char c, FILE *stream);
int uart_getltr(FILE *stream);
void cleanArray(unsigned char arr[10]);


#endif