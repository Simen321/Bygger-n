#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


uint8_t find_median_uint8(uint8_t arr[], int n); // directly googled
int compare_uint8(const void *a, const void *b);// directly googled

uint16_t utils_free_ram(void); // bytes left between the heap/static variables and the stack

#endif