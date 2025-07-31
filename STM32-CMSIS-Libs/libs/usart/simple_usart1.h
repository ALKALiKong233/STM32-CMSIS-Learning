#ifndef SIMPLE_USART
#define SIMPLE_USART
#include <stdint.h>

uint8_t usart1_init();
uint8_t usart1_send( void* buf, uint32_t len );
uint8_t usart1_receive( void* buf, uint32_t len );

// 仅在不使用 CMSIS Driver 时启用
//#define SIMPLE_USART_USES_INTERRUPT

#ifdef SIMPLE_USART_USES_INTERRUPT
// 中断相关的变量和函数声明
extern volatile uint8_t usart1_rx_buffer[256];
extern volatile uint32_t usart1_rx_index;
extern volatile uint8_t usart1_rx_complete;

uint8_t usart1_init_with_interrupt();  // 带中断的初始化
uint8_t usart1_receive_interrupt( void* buf, uint32_t* len );  // 中断接收

// 中断处理函数
void USART1_IRQHandler(void);
#endif

#endif