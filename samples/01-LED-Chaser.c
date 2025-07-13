#include "RTE_Components.h"
#include "libs/led/led_gpio.h"
#include <stdint.h>
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
int main() {

    // Enable GPIOs in RCC APB2ENR
    rcc_ptr->APB2ENR |= RCC_APB2ENR_IOPDEN;
    rcc_ptr->APB2ENR |= RCC_APB2ENR_IOPBEN;
    // Set the target gpio output
    for ( uint8_t i = 0; i < 4; ++i ) {
        led_init(led_list[i].bank, led_list[i].pin);
    }
    
    for (;;) {
        for ( uint8_t i = 0; i < 4; ++i ) {
            led_on(led_list[i].bank, led_list[i].pin);
            spin(1000000);
        }

        for ( uint8_t i = 0; i < 4; ++i ) {
            led_off(led_list[i].bank, led_list[i].pin);
            spin(1000000);
        }
    }
}
