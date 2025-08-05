#include "font.h"
#include <stddef.h>


/**
 * @brief 获取字符的点阵数据
 * @param c 要获取的字符
 * @return 指向字符点阵数据的指针，如果字符不支持则返回NULL
 */
const uint8_t* font_get_char_data(char c)
{
    // 检查字符是否在支持范围内
    if (c < FONT_ASCII_START || c > FONT_ASCII_END) {
        return NULL;
    }
    
    int index = c - ' ';
    
    return gsc_st7789_ascii_1608[index];
}
