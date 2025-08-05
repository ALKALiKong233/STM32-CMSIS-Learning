#ifndef DELAY_H
#define DELAY_H

#include "RTE_Components.h"
#include CMSIS_device_header
#include "stdint.h"
#include "stdbool.h"

void delay_init();
void SysTick_Handler();

uint32_t delay_get_tick();
void delay_ms( uint32_t ms );
void delay_us( uint16_t us );
bool timer_expired(uint32_t *t, uint32_t prd, uint32_t now);

#endif