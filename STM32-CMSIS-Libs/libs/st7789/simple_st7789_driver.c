#include "simple_st7789_driver.h"
#include "driver_st7789_interface.h"
#include "RTE_Components.h"
#include CMSIS_device_header

// 延时函数
static void st7789_delay_ms(uint32_t ms)
{
    st7789_interface_delay_ms(ms);
}

/**
 * @brief 发送命令到ST7789
 * @param cmd 命令字节
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_send_command(uint8_t cmd)
{
    uint8_t res;
    
    // 设置DC引脚为命令模式 (低电平)
    st7789_interface_cmd_data_gpio_write(0);
    
    // 发送命令
    res = st7789_interface_spi_write_cmd(&cmd, 1);
    return res;
}

/**
 * @brief 发送数据到ST7789
 * @param data 数据字节
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_send_data(uint8_t data)
{
    uint8_t res;
    
    // 设置DC引脚为数据模式 (高电平)
    st7789_interface_cmd_data_gpio_write(1);
    
    // 发送数据
    res = st7789_interface_spi_write_cmd(&data, 1);
    return res;
}

/**
 * @brief 发送16位数据到ST7789
 * @param data 16位数据
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_send_data_16(uint16_t data)
{
    uint8_t res;
    uint8_t data_bytes[2];
    
    // 高字节在前 (大端序)
    data_bytes[0] = (data >> 8) & 0xFF;
    data_bytes[1] = data & 0xFF;
    
    // 设置DC引脚为数据模式 (高电平)
    st7789_interface_cmd_data_gpio_write(1);
    
    // 发送数据
    res = st7789_interface_spi_write_cmd(data_bytes, 2);
    return res;
}

/**
 * @brief 设置显示窗口
 * @param x1 起始列
 * @param y1 起始行
 * @param x2 结束列
 * @param y2 结束行
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t res;
    
    // 设置列地址范围 (CASET)
    res = simple_st7789_send_command(ST7789_CASET);
    if (res != 0) return res;
    
    res = simple_st7789_send_data_16(x1);
    if (res != 0) return res;
    
    res = simple_st7789_send_data_16(x2);
    if (res != 0) return res;
    
    // 设置行地址范围 (RASET)
    res = simple_st7789_send_command(ST7789_RASET);
    if (res != 0) return res;
    
    res = simple_st7789_send_data_16(y1);
    if (res != 0) return res;
    
    res = simple_st7789_send_data_16(y2);
    if (res != 0) return res;
    
    return 0;
}

/**
 * @brief 初始化ST7789显示器
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_init(void)
{
    uint8_t res;
    
    // 1. 初始化硬件接口
    res = st7789_interface_backlight_gpio_init();
    if (res != 0) return 1;
    
    res = st7789_interface_cmd_data_gpio_init();
    if (res != 0) return 2;
    
    res = st7789_interface_reset_gpio_init();
    if (res != 0) return 3;
    
    res = st7789_interface_spi_init();
    if (res != 0) return 4;
    
    // 2. 开启背光
    st7789_interface_backlight_gpio_write(1);
    
    // 3. 硬件复位
    st7789_interface_reset_gpio_write(0);
    st7789_delay_ms(10);
    st7789_interface_reset_gpio_write(1);
    st7789_delay_ms(50);
    
    // 4. ST7789 初始化序列
    
    // 软件复位
    res = simple_st7789_send_command(ST7789_SWRESET);
    if (res != 0) return 5;
    st7789_delay_ms(150);
    
    // 退出睡眠模式
    res = simple_st7789_send_command(ST7789_SLPOUT);
    if (res != 0) return 6;
    st7789_delay_ms(10);
    
    // 设置颜色模式为RGB565
    res = simple_st7789_send_command(ST7789_COLMOD);
    if (res != 0) return 7;
    res = simple_st7789_send_data(0x05); // 16位RGB565
    if (res != 0) return 8;
    
    // 设置内存访问控制 (正常方向，RGB顺序)
    res = simple_st7789_send_command(ST7789_MADCTL);
    if (res != 0) return 9;
    res = simple_st7789_send_data(0x00); // 正常方向，RGB顺序
    if (res != 0) return 10;
    
    // 正常显示模式
    res = simple_st7789_send_command(ST7789_NORON);
    if (res != 0) return 11;
    st7789_delay_ms(10);

    // 反转颜色
    res = simple_st7789_send_command(ST7789_INVON);
    if (res != 0) return 13;
    st7789_delay_ms(10);

    // 开启显示
    res = simple_st7789_send_command(ST7789_DISPON);
    if (res != 0) return 12;
    st7789_delay_ms(100);
    
    return 0;
}

/**
 * @brief 反初始化ST7789
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_deinit(void)
{
    // 关闭显示
    simple_st7789_send_command(ST7789_DISPOFF);
    
    // 关闭背光
    st7789_interface_backlight_gpio_write(0);
    
    // 反初始化硬件接口
    st7789_interface_spi_deinit();
    st7789_interface_reset_gpio_deinit();
    st7789_interface_cmd_data_gpio_deinit();
    st7789_interface_backlight_gpio_deinit();
    
    return 0;
}

/**
 * @brief 填充整个屏幕
 * @param color RGB565颜色值
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_fill_screen(uint16_t color)
{
    return simple_st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

/**
 * @brief 填充矩形区域
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param width 宽度
 * @param height 高度
 * @param color RGB565颜色值
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    uint8_t res;
    uint32_t pixel_count = width * height;
    
    // 边界检查
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return 1;
    if (x + width > ST7789_WIDTH) width = ST7789_WIDTH - x;
    if (y + height > ST7789_HEIGHT) height = ST7789_HEIGHT - y;
    
    // 设置绘制窗口
    res = simple_st7789_set_window(x, y, x + width - 1, y + height - 1);
    if (res != 0) return res;
    
    // 开始写入像素数据
    res = simple_st7789_send_command(ST7789_RAMWR);
    if (res != 0) return res;
    
    // 发送颜色数据
    for (uint32_t i = 0; i < pixel_count; i++) {
        res = simple_st7789_send_data_16(color);
        if (res != 0) return res;
    }
    
    return 0;
}

/**
 * @brief 绘制单个像素
 * @param x X坐标
 * @param y Y坐标
 * @param color RGB565颜色值
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    return simple_st7789_fill_rect(x, y, 1, 1, color);
}

/**
 * @brief 简单的ST7789测试函数
 * @return 0=成功, 其他=失败
 */
uint8_t simple_st7789_test(void)
{
    uint8_t res;
    
    // 初始化显示器
    res = simple_st7789_init();
    if (res != 0) return res;
    
    // 测试1: 红色屏幕
    res = simple_st7789_fill_screen(COLOR_RED);
    if (res != 0) return res;
    st7789_delay_ms(1000);
    
    // 测试2: 绿色屏幕
    res = simple_st7789_fill_screen(COLOR_GREEN);
    if (res != 0) return res;
    st7789_delay_ms(1000);
    
    // 测试3: 蓝色屏幕
    res = simple_st7789_fill_screen(COLOR_BLUE);
    if (res != 0) return res;
    st7789_delay_ms(1000);
    
    // 测试4: 彩色矩形
    res = simple_st7789_fill_screen(COLOR_BLACK);
    if (res != 0) return res;
    
    res = simple_st7789_fill_rect(50, 50, 140, 100, COLOR_YELLOW);
    if (res != 0) return res;
    
    res = simple_st7789_fill_rect(70, 70, 100, 60, COLOR_CYAN);
    if (res != 0) return res;
    
    res = simple_st7789_fill_rect(90, 90, 60, 40, COLOR_MAGENTA);
    if (res != 0) return res;
    
    return 0;
}
