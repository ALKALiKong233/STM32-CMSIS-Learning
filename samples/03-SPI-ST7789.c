#include "RTE_Components.h"
#include CMSIS_device_header
#include "libs/st7789/simple_st7789_driver.h"

/**
 * @brief  ST7789显示测试函数
 * @return 状态码
 */
uint8_t st7789_test(void)
{
    uint8_t res;
    
    // 1. 初始化ST7789驱动
    res = simple_st7789_init();
    if (res != 0) {
        return 1;
    }
    
    // 2. 填充红色屏幕测试
    res = simple_st7789_fill_screen(COLOR_RED);
    if (res != 0) {
        return 1;
    }
    
    for (volatile uint32_t i = 0; i < 2000000; i++) __NOP(); // 观察2秒
    
    // 3. 矩形绘制测试
    res = simple_st7789_fill_rect(60, 60, 120, 100, COLOR_BLUE);
    if (res != 0) {
        return 1;
    }
    
    for (volatile uint32_t i = 0; i < 2000000; i++) __NOP(); // 观察2秒
    
    // 4. 主循环 - 6色循环显示
    uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA};
    uint8_t color_index = 0;
    
    while (1) {
        // 填充背景色
        simple_st7789_fill_screen(colors[color_index]);
        
        // 在中央添加对比色矩形
        uint16_t contrast_color = (color_index % 2 == 0) ? COLOR_WHITE : COLOR_BLACK;
        simple_st7789_fill_rect(80, 120, 80, 80, contrast_color);
        
        color_index = (color_index + 1) % 6;
        
        // 延时2秒
        for (volatile uint32_t i = 0; i < 20000000; i++) __NOP();
    }
    
    return 0;
}

int main() {
    // 启用必要的时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;

    // 开始ST7789测试
    st7789_test();

    for (;;) {
    }
}
