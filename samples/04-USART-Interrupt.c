#include "RTE_Components.h"
#include <stdint.h>
#include <string.h>
#include CMSIS_device_header

#include "libs/usart/simple_usart1.h"

int main() {

    // 使用带中断的USART初始化
    usart1_init_with_interrupt();
    
    // 发送欢迎消息
    const char* welcome = "USART1 Interrupt Demo Ready! Send data:\r\n";
    usart1_send((void*)welcome, strlen(welcome));

    uint8_t rx_buffer[256];
    uint32_t rx_len;

    for (;;) {
        // 检查是否有新数据通过中断接收
        rx_len = sizeof(rx_buffer);
        if (usart1_receive_interrupt(rx_buffer, &rx_len) == 0) {
            // 有新数据接收到
            
            // 发送确认消息
            const char* ack = "Received (interrupt): ";
            usart1_send((void*)ack, strlen(ack));
            
            // 回显接收到的数据
            usart1_send(rx_buffer, rx_len);
            
            // 添加换行
            const char* newline = "\r\n";
            usart1_send((void*)newline, 2);
        }
        
        // 主循环可以做其他事情，不需要阻塞等待串口数据
        // 这里可以添加其他任务
        for (volatile uint32_t i = 0; i < 10000; i++) {
            __NOP();  // 简单延时
        }
    }
}
