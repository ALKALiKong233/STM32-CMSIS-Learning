#ifndef LED_GPIO
#define LED_GPIO

#include <stdint.h>
#include "RTE_Components.h"
#include CMSIS_device_header

/* Init the LED GPIO Port output */
static inline void led_init(const char bank, const uint8_t pin) {
    GPIO_TypeDef *gpio = (GPIO_TypeDef *) ( GPIOA_BASE + 0x400 * ( bank - 'A' ) );
    /* Set mode bits in CRL/CRH
     * 00: Input mode (reset state)
     * 01: Output mode, max speed 10 MHz.
     * 10: Output mode, max speed 2 MHz.
     * 11: Output mode, max speed 50 MHz.
     * For LED devices, simply 2 MHz output is enough.
     * Higher freq means higher drains and transmission speed.
    */
    /* Set configuration bits in CRL/CRH
     * In input mode (MODE[1:0]=00):
     *  00: Analog mode
     *  01: Floating input (reset state)
     *  10: Input with pull-up / pull-down
     *  11: Reserved
     * In output mode (MODE[1:0] > 00):
     *  00: General purpose output push-pull
     *  01: General purpose output Open-drain
     *  10: Alternate function output Push-pull
     *  11: Alternate function output Open-drain
     * For LED devices, we use push-pull output.
    */
    if ( pin < 8 ) {
        // Reset the bits before setting the bits.
        uint32_t pos = pin * 4;
        gpio->CRL &= ~(0xF << pos);        // Clear 4 bits (1111 binary)
        gpio->CRL |= (0x2 << pos);         // 0x2 (hex) -> 0010 (binary)
    } else if ( pin < 16 ) {
        // Reset the bits before setting the bits.
        uint32_t pos = (pin - 8) * 4;
        gpio->CRH &= ~(0xF << pos);        // Clear 4 bits (1111 binary)
        gpio->CRH |= (0x2 << pos);         // 0x2 (hex) -> 0010 (binary)
    }
}

/* LED ON */
static inline void led_on(const char bank, const uint8_t pin) {
    /* From the document:
     * Note: For atomic bit set/reset, the ODR bits can be individually set and cleared by writing to 
the GPIOx_BSRR register (x = A .. G).
     * We get to know that we can set BSRR to operate ODR.
     * Bits 31:16 BRy: Port x Reset bit y (y= 0 .. 15)
     * Bits 15:0  BSy: Port x Set bit y (y= 0 .. 15)
     * Note: If both BSx and BRx are set, BSx has priority.
    */
    GPIO_TypeDef *gpio = (GPIO_TypeDef *) ( GPIOA_BASE + 0x400 * ( bank - 'A' ) );
    /* Our LED is common anode LED, so reset means turn it on. */
    gpio->BSRR = (1 << (pin + 16));
}

/* LED OFF */
static inline void led_off(const char bank, const uint8_t pin) {
    GPIO_TypeDef *gpio = (GPIO_TypeDef *) ( GPIOA_BASE + 0x400 * ( bank - 'A' ) );
    gpio->BSRR = (1 << pin);
}

#endif