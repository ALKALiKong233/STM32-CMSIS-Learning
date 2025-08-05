#ifndef __DHT11_H
#define __DHT11_H

#include "stdint.h"
#include "RTE_Components.h"
#include CMSIS_device_header

// DHT11 GPIO
#define DHT11_GPIO_PORT     GPIOC
#define DHT11_PIN_NUM       4
#define DHT11_GPIO_PIN      (1 << DHT11_PIN_NUM)
#define DHT11_GPIO_CLK      RCC_APB2ENR_IOPCEN


// DHT11数据结构
typedef struct {
    uint8_t humity;     // 湿度整数部分
    uint8_t temp;       // 温度整数部分
    uint8_t humity_dec; // 湿度小数部分 (DHT11固定为0)
    uint8_t temp_dec;   // 温度小数部分
    uint8_t check;      // 校验和
    _Bool negative;     // 是否为负数
} dht11_dt;

// Set the PIN to Input with pull-up / pull-down Mode ( 0x8 -> 0b1000 )
#define DHT11_IO_IN()   do { \
    if(DHT11_PIN_NUM < 8) { \
        DHT11_GPIO_PORT->CRL &= ~(0xF << (DHT11_PIN_NUM * 4)); \
        DHT11_GPIO_PORT->CRL |= (0x8 << (DHT11_PIN_NUM * 4)); \
    } else { \
        DHT11_GPIO_PORT->CRH &= ~(0xF << ((DHT11_PIN_NUM - 8) * 4)); \
        DHT11_GPIO_PORT->CRH |= (0x8 << ((DHT11_PIN_NUM - 8) * 4)); \
    } \
} while(0)

// Set the PIN to Push-pull Output Mode ( 0x3 -> 0b0011 )
#define DHT11_IO_OUT()  do { \
    if(DHT11_PIN_NUM < 8) { \
        DHT11_GPIO_PORT->CRL &= ~(0xF << (DHT11_PIN_NUM * 4)); \
        DHT11_GPIO_PORT->CRL |= (0x3 << (DHT11_PIN_NUM * 4)); \
    } else { \
        DHT11_GPIO_PORT->CRH &= ~(0xF << ((DHT11_PIN_NUM - 8) * 4)); \
        DHT11_GPIO_PORT->CRH |= (0x3 << ((DHT11_PIN_NUM - 8) * 4)); \
    } \
} while(0)

// Set the output of the DHT11 PIN
#define DHT11_DQ_OUT(x)  do { \
    if(x) DHT11_GPIO_PORT->BSRR = DHT11_GPIO_PIN; \
    else DHT11_GPIO_PORT->BRR = DHT11_GPIO_PIN; \
} while(0)

// Read the input of the DHT11 PIN
#define DHT11_DQ_IN      (DHT11_GPIO_PORT->IDR & DHT11_GPIO_PIN)


uint8_t dht11_init(void);
/**
 * @brief  Check the connectivity between DHT11 and MCU.
 * @return status code
 *         - 0 Success.
 *         - 1 Timed out.
 */
uint8_t dht11_check(void);

/**
 * @brief  Read data from DHT11
 * @param datavalue A pointer to the dest where receives the data.
 * @return status code
 *         - 0 Success.
 *         - 1 Failed to connect with DHT11.
 *         - 2 Checksum failed.
 */
uint8_t dht11_read(dht11_dt *datavalue);
void dht11_rst(void);

#endif /* __DHT11_H */
