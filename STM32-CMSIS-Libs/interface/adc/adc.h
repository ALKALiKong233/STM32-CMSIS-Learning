#ifndef INTERFACE_ADC_H
#define INTERFACE_ADC_H
#include "stdint.h"

typedef enum _ADC_CHANNEL {
    ADC_CH0_PA0,
    ADC_CH1_PA1,
    ADC_CH2_PA2,
    ADC_CH3_PA3,
    ADC_CH4_PA4,
    ADC_CH5_PA5,
    ADC_CH6_PA6,
    ADC_CH7_PA7,
    ADC_CH8_PB0,
    ADC_CH9_PB1,
    ADC_CH10_PC0,
    ADC_CH11_PC1,
    ADC_CH12_PC2,
    ADC_CH13_PC3,
    ADC_CH14_PC4,
    ADC_CH15_PC5
} ADC_CHANNEL;

uint8_t adc_init(ADC_CHANNEL ch);
uint16_t adc_get_single(ADC_CHANNEL ch);

#endif