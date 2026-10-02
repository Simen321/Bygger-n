#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define ADC_ADDR ((volatile uint8_t *)0x1C00)


void adc_init();
void adc_clock_init(void);
uint8_t adc_read(uint8_t channel);

void adc_print_all();
void lets_test_the_fucking_adc(void);

#endif