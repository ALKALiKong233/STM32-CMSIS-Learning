#include "simple_usart1.h"
#include "stm32f10x.h"
#include <stdint.h>

#ifdef SIMPLE_USART_USES_INTERRUPT
volatile uint8_t usart1_rx_buffer[256];
volatile uint32_t usart1_rx_index = 0;
volatile uint8_t usart1_rx_complete = 0;
#endif

uint8_t usart1_init() {
    /* Enable RCC */
    // Enable AFIOEN
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    // Enable IOPAEN
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    // Enable USART1EN
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* Configure GPIO pins for USART1 */
    // PA9 (TX) -  Alternate function push-pull (50MHz)
    // 清除PA9配置位 (CRH bits 7:4)
    GPIOA->CRH &= ~(0xF << 4);
    // 设置PA9为复用推挽输出，50MHz (1011)
    GPIOA->CRH |= (0xB << 4);
    
    // PA10 (RX) - Input floating / Input pull-up
    // 清除PA10配置位 (CRH bits 11:8)  
    GPIOA->CRH &= ~(0xF << 8);
    // 设置PA10为浮空输入 (0100)
    GPIOA->CRH |= (0x4 << 8);

    /* Configure USART1 */
    // 配置波特率
    // 115200 -> 39.0625
    // 小数部分 = 0.0625 * 16 = 1
    USART1->BRR = (39 << 4) | 1;
    
    // 配置控制寄存器
    // 先清除所有配置，然后设置需要的位
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    
    // CR1配置:
    // UE (bit 13): USART Enable - 最后设置
    // M (bit 12): 0 = 8 data bits
    // PCE (bit 10): 0 = No parity
    // TE (bit 3): Transmitter enable
    // RE (bit 2): Receiver enable
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
    
    // CR2配置:
    // STOP[13:12]: 00 = 1 stop bit (默认值)
    // 前面已经被置零了，无需设置

    // 最后启用USART
    USART1->CR1 |= USART_CR1_UE;

    return 0;
}

#ifdef SIMPLE_USART_USES_INTERRUPT
uint8_t usart1_init_with_interrupt() {
    /* Enable RCC */
    // Enable AFIOEN
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    // Enable IOPAEN
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    // Enable USART1EN
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* Configure GPIO pins for USART1 */
    // PA9 (TX) -  Alternate function push-pull (50MHz)
    // 清除PA9配置位 (CRH bits 7:4)
    GPIOA->CRH &= ~(0xF << 4);
    // 设置PA9为复用推挽输出，50MHz (1011)
    GPIOA->CRH |= (0xB << 4);
    
    // PA10 (RX) - Input floating / Input pull-up
    // 清除PA10配置位 (CRH bits 11:8)  
    GPIOA->CRH &= ~(0xF << 8);
    // 设置PA10为浮空输入 (0100)
    GPIOA->CRH |= (0x4 << 8);

    /* Configure USART1 */
    // 配置波特率
    // 115200 -> 39.0625
    // 小数部分 = 0.0625 * 16 = 1
    USART1->BRR = (39 << 4) | 1;
    
    // 配置控制寄存器
    // 先清除所有配置，然后设置需要的位
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    
    // CR1配置:
    // UE (bit 13): USART Enable - 最后设置
    // M (bit 12): 0 = 8 data bits
    // PCE (bit 10): 0 = No parity
    // TE (bit 3): Transmitter enable
    // RE (bit 2): Receiver enable
    // RXNEIE (bit 5): RX Not Empty Interrupt Enable - 新增
    // IDLEIE (bit 4): IDLE Interrupt Enable - 新增
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_IDLEIE;
    
    // CR2配置:
    // STOP[13:12]: 00 = 1 stop bit (默认值)
    // 前面已经被置零了，无需设置

    // 配置NVIC - 启用USART1中断
    NVIC_SetPriority(USART1_IRQn, 1);  // 设置中断优先级
    NVIC_EnableIRQ(USART1_IRQn);       // 启用USART1中断

    // 最后启用USART
    USART1->CR1 |= USART_CR1_UE;

    // 初始化中断相关变量
    usart1_rx_index = 0;
    usart1_rx_complete = 0;

    return 0;
}
#endif

uint8_t usart1_send(void* buf, uint32_t len) {
    uint8_t* data = (uint8_t*)buf;
    
    for (uint32_t i = 0; i < len; i++) {
        while (!(USART1->SR & USART_SR_TXE)) {}
        
        USART1->DR = data[i];
    }
    
    while (!(USART1->SR & USART_SR_TC)) {}
    
    return 0;
}

uint8_t usart1_receive(void* buf, uint32_t len) {
    uint8_t* data = (uint8_t*)buf;
    uint32_t _size = 0;
    volatile uint32_t timeout = 100;
    // 等待接收数据寄存器非空
    while ( !(USART1->SR & USART_SR_RXNE) && timeout > 0 ) {
        timeout--;
    }
    if ( USART1->SR & USART_SR_RXNE )
        *data = (uint8_t)(USART1->DR & 0xFF);
    else
        return 1;
    
    return 0;
}

#ifdef SIMPLE_USART_USES_INTERRUPT
uint8_t usart1_receive_interrupt(void* buf, uint32_t* len) {
    uint8_t* data = (uint8_t*)buf;
    
    if (usart1_rx_complete) {
        // 复制接收到的数据
        for (uint32_t i = 0; i < usart1_rx_index && i < *len; i++) {
            data[i] = usart1_rx_buffer[i];
        }
        
        *len = usart1_rx_index;  // 返回实际接收的字节数
        
        // 重置状态
        usart1_rx_index = 0;
        usart1_rx_complete = 0;
        
        return 0;  // 成功
    }
    
    *len = 0;
    return 1;  // 没有数据
}

// 硬件中断处理函数
void USART1_IRQHandler(void) {
    uint32_t sr = USART1->SR;
    
    // 检查接收数据寄存器非空中断
    if (sr & USART_SR_RXNE) {
        // 读取接收到的数据
        uint8_t received_data = (uint8_t)(USART1->DR & 0xFF);
        
        // 将数据存储到缓冲区（防止溢出）
        if (usart1_rx_index < sizeof(usart1_rx_buffer)) {
            usart1_rx_buffer[usart1_rx_index++] = received_data;
        }

        // 如果读入的数据数量大于缓冲区，也设置接收完成
        // 其实这里最好是塞一个解决溢出的方案，不过目前我还不是很熟悉这个内存管理机制，就先不做了
        if (usart1_rx_index >= sizeof(usart1_rx_buffer)) {
            usart1_rx_complete = 1;
        }
    }
    
    // 检查空闲线路中断（接收完成）
    if (sr & USART_SR_IDLE) {
        // 读取DR寄存器清除IDLE标志
        USART1->DR;
        
        // 设置接收完成标志
        usart1_rx_complete = 1;
    }
    
    // 检查溢出错误
    if (sr & USART_SR_ORE) {
        USART1->DR;
    }
    
    // 检查帧错误
    if (sr & USART_SR_FE) {
        USART1->DR;
    }
    
    // 检查校验错误
    if (sr & USART_SR_PE) {
        USART1->DR;
    }
}
#endif
