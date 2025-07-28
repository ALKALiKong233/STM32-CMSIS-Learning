#ifndef SIMPLE_ST7789_DRIVER_H
#define SIMPLE_ST7789_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// ST7789 显示屏尺寸定义
#define ST7789_WIDTH    240
#define ST7789_HEIGHT   320

// RGB565 颜色定义
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F

// ST7789 常用命令定义
#define ST7789_SWRESET  0x01  // 软件复位
#define ST7789_SLPOUT   0x11  // 退出睡眠
#define ST7789_NORON    0x13  // 正常显示模式
#define ST7789_INVOFF   0x20  // 显示反转关闭
#define ST7789_INVON    0x21  // 显示反转开启
#define ST7789_DISPOFF  0x28  // 显示关闭
#define ST7789_DISPON   0x29  // 显示开启
#define ST7789_CASET    0x2A  // 列地址设置
#define ST7789_RASET    0x2B  // 行地址设置
#define ST7789_RAMWR    0x2C  // 内存写入
#define ST7789_MADCTL   0x36  // 内存访问控制
#define ST7789_COLMOD   0x3A  // 像素格式设置

// 内存访问控制 (MADCTL) 位定义
#define ST7789_MADCTL_MY    0x80  // 行地址顺序
#define ST7789_MADCTL_MX    0x40  // 列地址顺序
#define ST7789_MADCTL_MV    0x20  // 行/列交换
#define ST7789_MADCTL_ML    0x10  // 垂直刷新顺序
#define ST7789_MADCTL_BGR   0x08  // RGB/BGR顺序
#define ST7789_MADCTL_MH    0x04  // 水平刷新顺序

// 函数声明
uint8_t simple_st7789_init(void);
uint8_t simple_st7789_deinit(void);
uint8_t simple_st7789_fill_screen(uint16_t color);
uint8_t simple_st7789_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
uint8_t simple_st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
uint8_t simple_st7789_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
uint8_t simple_st7789_send_command(uint8_t cmd);
uint8_t simple_st7789_send_data(uint8_t data);
uint8_t simple_st7789_send_data_16(uint16_t data);

#endif // SIMPLE_ST7789_DRIVER_H
