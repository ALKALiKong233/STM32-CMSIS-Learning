#include "RTE_Components.h"
#include "libs/led/led_gpio.h"
#include <stdint.h>
#include <stdbool.h>
#include CMSIS_device_header

RCC_TypeDef *rcc_ptr = (RCC_TypeDef *)RCC_BASE;
struct LED_INFO {
    char bank;
    uint8_t pin;
} led_list[4] = { {'D', 12}, {'D', 11},
                    {'D', 9}, {'B', 8} };
static inline void spin( volatile uint32_t tick ) {
    while ( tick-- ) {
        asm("nop");
    }
}

/* SysTick Settings */
// Important: the variable is needed to be set as volatile
volatile uint32_t systick_counter = 0;
/* SysTick is a global variable (ptr) */
static inline void systick_init() {
    SysTick->CTRL |= SysTick_CTRL_ENABLE |
                    SysTick_CTRL_TICKINT |
                    SysTick_CTRL_CLKSOURCE;
    /* Our MCU is at 72MHz, so set load to this for 1ms timer */
    SysTick->LOAD = 72000 - 1;
    SysTick->VAL = 0;
}
/* SysTick_Handler(void) is called when SysTick->VAL dropped from 1 to 0 */
void SysTick_Handler(void) {
    systick_counter++;
}
// t: expiration time, prd: period, now: current time. Return true if expired
bool timer_expired(uint32_t *t, uint32_t prd, uint32_t now) {
    if (now + prd < *t) *t = 0;                    // Time wrapped? Reset timer
    if (*t == 0) *t = now + prd;                   // First poll? Set expiration
    if (*t > now) return false;                    // Not expired yet, return
    *t = (now - *t) > prd ? now + prd : *t + prd;  // Next expiration time
    return true;                                   // Expired, return true
}

int main() {

    // Enable GPIOs in RCC APB2ENR
    rcc_ptr->APB2ENR |= RCC_APB2ENR_IOPDEN;
    rcc_ptr->APB2ENR |= RCC_APB2ENR_IOPBEN;
    // Set the target gpio output
    for ( uint8_t i = 0; i < 4; ++i ) {
        led_init(led_list[i].bank, led_list[i].pin);
    }
    
    // Initialize SysTick timer
    systick_init();
    
    uint32_t timer = 0, period = 500;
    volatile bool led_enabled = true;
    for (;;) {
        if (timer_expired(&timer, period, systick_counter)) {
            for ( uint8_t i = 0; i < 4; ++i ) {
                if ( led_enabled )
                    led_off(led_list[i].bank, led_list[i].pin);
                else
                    led_on(led_list[i].bank, led_list[i].pin);
            }
            led_enabled = !led_enabled;
        }
    }
}
