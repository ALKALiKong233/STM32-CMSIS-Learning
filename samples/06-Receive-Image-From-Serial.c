#include "RTE_Components.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include CMSIS_device_header
#include "libs/console/console.h"
#include "libs/delay/delay.h"
#include "libs/st7789/simple_st7789_driver.h"

#define ROWS_IN_A_CHUNK 4

void st7789_read_chunks() {
    uint8_t buf[MAX_CHUNK_SIZE];
    uint32_t buf_len;
    simple_st7789_set_window(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
    simple_st7789_send_command(ST7789_RAMWR);

    for ( uint16_t i = 0; i < ST7789_HEIGHT / ROWS_IN_A_CHUNK; ++i ) {
        snprintf(buf, 2048, "READY_CHUNK_%d\n", i);
        console_info(buf, strlen(buf));
        while ( console_read(buf, &buf_len) != CONSOLE_READ_OK );
        simple_st7789_send_data_buf(buf, buf_len);
        snprintf(buf, 2048, "CHUNK_%d_OK, len: %d\n", i, buf_len);
        console_info(buf, strlen(buf));
        
    }
}
int main() {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
    delay_init();
    console_init();
    simple_st7789_init();

    // Key 1 -> PC1
    GPIOC->CRL &= ~(0b1111 << 4);
    GPIOC->CRL |= (0b0100 << 4);

    for (;;) {
        if ( !( GPIOC->IDR & (1<<1) ) ) {
            static const char* ready_msg = "IMAGE_RECEIVER_READY\n";
            console_info(ready_msg, strlen(ready_msg));
            simple_st7789_fill_screen(COLOR_WHITE);
            simple_st7789_draw_string(5, 5, "You can send the color data now.", COLOR_BLACK, COLOR_WHITE);
            st7789_read_chunks();
        }
    }
}
