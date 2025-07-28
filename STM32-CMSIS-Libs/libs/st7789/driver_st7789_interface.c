/**
 *Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_st7789_interface_template.c
 * @brief     driver st7789 interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/04/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_st7789_interface.h"
#include "Driver_Common.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdint.h>

extern ARM_DRIVER_SPI Driver_SPI1;

// ST7789 GPIO引脚定义
#define ST7789_CS_PORT   'E'    // 片选引脚 PE1
#define ST7789_CS_PIN    1
#define ST7789_DC_PORT   'E'    // 数据/命令引脚 PE0  
#define ST7789_DC_PIN    0
#define ST7789_RST_PORT  'E'    // 复位引脚 PE3
#define ST7789_RST_PIN   3
#define ST7789_BL_PORT   'A'    // 背光引脚 PA8
#define ST7789_BL_PIN    8

// GPIO操作宏定义
#define GPIO_PORT(bank) ((GPIO_TypeDef *)(GPIOA_BASE + 0x400 * ((bank) - 'A')))
#define GPIO_SET(port, pin) (port->BSRR = (1 << pin))
#define GPIO_RESET(port, pin) (port->BSRR = (1 << (pin + 16)))

// 初始化GPIO引脚为推挽输出模式
static void gpio_init_output(char bank, uint8_t pin) {
    GPIO_TypeDef *gpio = GPIO_PORT(bank);
    
    if (pin < 8) {
        uint32_t pos = pin * 4;
        gpio->CRL &= ~(0xF << pos);  // 清除配置位
        gpio->CRL |= (0x2 << pos);   // 设置为2MHz输出，推挽模式
    } else if (pin < 16) {
        uint32_t pos = (pin - 8) * 4;
        gpio->CRH &= ~(0xF << pos);  // 清除配置位
        gpio->CRH |= (0x2 << pos);   // 设置为2MHz输出，推挽模式
    }
}

// SPI事件回调函数
void SPI1_Event_Callback(uint32_t event) {
    if (event & ARM_SPI_EVENT_TRANSFER_COMPLETE) {
        // SPI传输完成
    }
    if (event & ARM_SPI_EVENT_DATA_LOST) {
        // 数据丢失错误
    }
    if (event & ARM_SPI_EVENT_MODE_FAULT) {
        // 模式错误
    }
}

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t st7789_interface_spi_init(void)
{
    int32_t status;

    // 初始化片选引脚
    gpio_init_output(ST7789_CS_PORT, ST7789_CS_PIN);
    GPIO_TypeDef *cs_port = GPIO_PORT(ST7789_CS_PORT);
    GPIO_SET(cs_port, ST7789_CS_PIN); // 默认拉高片选

    status = Driver_SPI1.Initialize(SPI1_Event_Callback);
    if (status != ARM_DRIVER_OK) {
        return 1;
    }

    status = Driver_SPI1.PowerControl(ARM_POWER_FULL);
    if (status != ARM_DRIVER_OK) {
        return 1;
    }

    status = Driver_SPI1.Control(ARM_SPI_MODE_MASTER |       // 主模式
                                ARM_SPI_CPOL0_CPHA0 |        // 时钟极性和相位
                                ARM_SPI_SS_MASTER_SW |       // 软件控制片选
                                ARM_SPI_DATA_BITS(8),        // 8位数据
                                36000000);
    if (status != ARM_DRIVER_OK) {
        return 1;
    }

    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t st7789_interface_spi_deinit(void)
{
    int32_t status;

    // 先关闭SPI电源
    status = Driver_SPI1.PowerControl(ARM_POWER_OFF);
    if (status != ARM_DRIVER_OK) {
        return 1;
    }

    // 然后去初始化SPI驱动
    status = Driver_SPI1.Uninitialize();
    if (status != ARM_DRIVER_OK) {
        return 1;
    }

    // 将片选引脚设置为输入模式
    GPIO_TypeDef *gpio = GPIO_PORT(ST7789_CS_PORT);
    if (ST7789_CS_PIN < 8) {
        uint32_t pos = ST7789_CS_PIN * 4;
        gpio->CRL &= ~(0xF << pos);  // 清除配置位，默认为浮空输入
    } else if (ST7789_CS_PIN < 16) {
        uint32_t pos = (ST7789_CS_PIN - 8) * 4;
        gpio->CRH &= ~(0xF << pos);  // 清除配置位，默认为浮空输入
    }

    return 0;
}

/**
 * @brief     interface spi bus write
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t st7789_interface_spi_write_cmd(uint8_t *buf, uint16_t len)
{
    int32_t status;
    GPIO_TypeDef *cs_port = GPIO_PORT(ST7789_CS_PORT);
    
    // 拉低片选
    GPIO_RESET(cs_port, ST7789_CS_PIN);
    
    // 发送数据
    status = Driver_SPI1.Send(buf, len);
    if (status != ARM_DRIVER_OK) {
        GPIO_SET(cs_port, ST7789_CS_PIN); // 失败时释放片选
        return 1;
    }
    
    // 等待传输完成
    while (Driver_SPI1.GetStatus().busy);
    
    // 拉高片选
    GPIO_SET(cs_port, ST7789_CS_PIN);
    
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void st7789_interface_delay_ms(uint32_t ms)
{
    // 使用简单的循环延时，避免与主程序的SysTick冲突
    // 假设系统时钟为72MHz，大概的循环次数
    volatile uint32_t count = ms * 18000; // 大约1ms的延时
    
    while (count--) {
        __NOP(); // ARM的空操作指令
    }
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void st7789_interface_debug_print(const char *const fmt, ...)
{
    (void)fmt; // 避免未使用参数的警告
    
    // 之后实现 stdio 重定向至 UART 后会在这里实现输出 debug 信息的功能
}

/**
 * @brief  interface command && data gpio init
 * @return status code
 *         - 0 success
 *         - 1 gpio init failed
 * @note   none
 */
uint8_t st7789_interface_cmd_data_gpio_init(void)
{
    // 初始化DC引脚为输出模式
    gpio_init_output(ST7789_DC_PORT, ST7789_DC_PIN);
    
    // 设置默认状态为命令模式 (DC = 0)
    GPIO_TypeDef *dc_port = GPIO_PORT(ST7789_DC_PORT);
    GPIO_RESET(dc_port, ST7789_DC_PIN);
    
    return 0;
}

/**
 * @brief  interface command && data gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 gpio deinit failed
 * @note   none
 */
uint8_t st7789_interface_cmd_data_gpio_deinit(void)
{
    // 将DC引脚设置为输入模式（浮空输入）
    GPIO_TypeDef *gpio = GPIO_PORT(ST7789_DC_PORT);
    
    if (ST7789_DC_PIN < 8) {
        uint32_t pos = ST7789_DC_PIN * 4;
        gpio->CRL &= ~(0xF << pos);  // 清除配置位，默认为浮空输入
    } else if (ST7789_DC_PIN < 16) {
        uint32_t pos = (ST7789_DC_PIN - 8) * 4;
        gpio->CRH &= ~(0xF << pos);  // 清除配置位，默认为浮空输入
    }
    
    return 0;
}

/**
 * @brief     interface command && data gpio write
 * @param[in] value written value
 * @return    status code
 *            - 0 success
 *            - 1 gpio write failed
 * @note      none
 */
uint8_t st7789_interface_cmd_data_gpio_write(uint8_t value)
{
    GPIO_TypeDef *dc_port = GPIO_PORT(ST7789_DC_PORT);
    
    if (value) {
        // 数据模式 (DC = 1)
        GPIO_SET(dc_port, ST7789_DC_PIN);
    } else {
        // 命令模式 (DC = 0)
        GPIO_RESET(dc_port, ST7789_DC_PIN);
    }
    
    return 0;
}

/**
 * @brief  interface reset gpio init
 * @return status code
 *         - 0 success
 *         - 1 gpio init failed
 * @note   none
 */
uint8_t st7789_interface_reset_gpio_init(void)
{
    // 初始化RST引脚为输出模式
    gpio_init_output(ST7789_RST_PORT, ST7789_RST_PIN);
    
    // 设置默认状态为高电平（非复位状态）
    GPIO_TypeDef *rst_port = GPIO_PORT(ST7789_RST_PORT);
    GPIO_SET(rst_port, ST7789_RST_PIN);
    
    return 0;
}

/**
 * @brief  interface reset gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 gpio deinit failed
 * @note   none
 */
uint8_t st7789_interface_reset_gpio_deinit(void)
{
    // 将RST引脚设置为输入模式（浮空输入）
    GPIO_TypeDef *gpio = GPIO_PORT(ST7789_RST_PORT);
    
    if (ST7789_RST_PIN < 8) {
        uint32_t pos = ST7789_RST_PIN * 4;
        gpio->CRL &= ~(0xF << pos);  // 清除配置位，默认为浮空输入
    } else if (ST7789_RST_PIN < 16) {
        uint32_t pos = (ST7789_RST_PIN - 8) * 4;
        gpio->CRH &= ~(0xF << pos);  // 清除配置位，默认为浮空输入
    }
    
    return 0;
}

/**
 * @brief     interface reset gpio write
 * @param[in] value written value
 * @return    status code
 *            - 0 success
 *            - 1 gpio write failed
 * @note      none
 */
uint8_t st7789_interface_reset_gpio_write(uint8_t value)
{
    GPIO_TypeDef *rst_port = GPIO_PORT(ST7789_RST_PORT);
    
    if (value) {
        // 非复位状态 (RST = 1)
        GPIO_SET(rst_port, ST7789_RST_PIN);
    } else {
        // 复位状态 (RST = 0)
        GPIO_RESET(rst_port, ST7789_RST_PIN);
    }
    
    return 0;
}

/**
 * @brief  interface backlight gpio init
 * @return status code
 *         - 0 success
 *         - 1 gpio init failed
 * @note   none
 */
uint8_t st7789_interface_backlight_gpio_init(void)
{
    // 初始化背光控制引脚为推挽输出模式
    gpio_init_output(ST7789_BL_PORT, ST7789_BL_PIN);
    
    return 0;
}

/**
 * @brief  interface backlight gpio deinit  
 * @return status code
 *         - 0 success
 *         - 1 gpio deinit failed
 * @note   none
 */
uint8_t st7789_interface_backlight_gpio_deinit(void)
{
    // 关闭背光
    st7789_interface_backlight_gpio_write(0);
    
    return 0;
}

/**
 * @brief     interface backlight gpio write
 * @param[in] value written value (1=on, 0=off)
 * @return    status code
 *            - 0 success
 *            - 1 gpio write failed
 * @note      none
 */
uint8_t st7789_interface_backlight_gpio_write(uint8_t value)
{
    GPIO_TypeDef *bl_port = GPIO_PORT(ST7789_BL_PORT);
    
    if (value) {
        // 开启背光 (BL = 1)
        GPIO_SET(bl_port, ST7789_BL_PIN);
    } else {
        // 关闭背光 (BL = 0)
        GPIO_RESET(bl_port, ST7789_BL_PIN);
    }
    
    return 0;
}
