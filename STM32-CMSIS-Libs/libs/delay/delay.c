#include "delay.h"
#include <stdint.h>

volatile uint32_t systick_counter = 0;

void delay_init() {
    SysTick->CTRL |= SysTick_CTRL_ENABLE |
                    SysTick_CTRL_TICKINT |
                    SysTick_CTRL_CLKSOURCE;
    /* Our MCU is at 72MHz, so set load to this for 1ms timer */
    SysTick->LOAD = 72000 - 1;
    SysTick->VAL = 0;
}

void SysTick_Handler() {
    ++systick_counter;
}

uint32_t delay_get_tick() {
    return systick_counter;
}

void delay_ms(uint32_t ms) {
    uint32_t start = systick_counter;
    while (systick_counter - start < ms);
    return;
}

// t: expiration time, prd: period, now: current time. Return true if expired
bool timer_expired(uint32_t *t, uint32_t prd, uint32_t now) {
    if (now + prd < *t) *t = 0;                    // Time wrapped? Reset timer
    if (*t == 0) *t = now + prd;                   // First poll? Set expiration
    if (*t > now) return false;                    // Not expired yet, return
    *t = (now - *t) > prd ? now + prd : *t + prd;  // Next expiration time
    return true;                                   // Expired, return true
}
