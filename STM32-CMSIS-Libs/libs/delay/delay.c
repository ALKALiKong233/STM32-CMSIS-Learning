#include "delay.h"
#include "hal/lv_hal_tick.h"
#include "stm32f10x.h"
#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

/* SysTick */
volatile uint32_t systick_counter = 0;
volatile bool is_delay_inited = false;

void delay_init() {
    if ( is_delay_inited ) return;
    /* SysTick Init */
    SysTick->CTRL |= SysTick_CTRL_ENABLE |
                    SysTick_CTRL_TICKINT |
                    SysTick_CTRL_CLKSOURCE;
    /* Our MCU is at 72MHz, so set load to this for 1ms timer */
    SysTick->LOAD = 72000 - 1;
    SysTick->VAL = 0;

    /* TIM6 Init */
    // 使能TIM6时钟
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    // 重置TIM6并清除重置位
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM6RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM6RST;
    
    // 配置TIM6
    TIM6->PSC = 72 - 1;
    TIM6->ARR = 0xFFFF;
    TIM6->CNT = 0;

    // 启动定时器
    TIM6->CR1 |= TIM_CR1_CEN;

    is_delay_inited = true;
}

void SysTick_Handler() {
    ++systick_counter;
    lv_tick_inc(1);
}

uint32_t delay_get_tick() {
    if ( !is_delay_inited ) delay_init();
    return systick_counter;
}

void delay_ms(uint32_t ms) {
    if ( !is_delay_inited ) delay_init();
    uint32_t start = systick_counter;
    while (systick_counter - start < ms);
    return;
}

// t: expiration time, prd: period, now: current time. Return true if expired
bool timer_expired(uint32_t *t, uint32_t prd, uint32_t now) {
    if ( !is_delay_inited ) delay_init();
    if (now + prd < *t) *t = 0;                    // Time wrapped? Reset timer
    if (*t == 0) *t = now + prd;                   // First poll? Set expiration
    if (*t > now) return false;                    // Not expired yet, return
    *t = (now - *t) > prd ? now + prd : *t + prd;  // Next expiration time
    return true;                                   // Expired, return true
}

void delay_us(uint16_t us) {
    if ( !is_delay_inited ) delay_init();
    if (us == 0) return;

    uint16_t start = TIM6->CNT;
    uint16_t target = start + us;
    
    // 检查是否会发生溢出
    if (target > start) {
        // 无溢出情况：直接等待
        while (TIM6->CNT < target && TIM6->CNT >= start);
    } else {
        // 会发生溢出：先等到溢出，再等到目标值
        while (TIM6->CNT >= start);  // 等待溢出
        while (TIM6->CNT < target);  // 等待到目标值
    }
}
