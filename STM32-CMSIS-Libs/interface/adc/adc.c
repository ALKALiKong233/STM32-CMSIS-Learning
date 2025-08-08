#include "adc.h"
#include "stdint.h"
#include "stm32f10x.h"

static void adc_config_init( ) {
    static _Bool finished = 0;
    if ( finished ) return;

    // 配置ADC时钟分频 (PCLK2/6 = 72MHz/6 = 12MHz)
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;

    // 复位ADC1配置
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;

    // 配置ADC为独立模式，12位分辨率
    ADC1->CR1 &= ~ADC_CR1_DUALMOD;  // 独立模式
    
    // 配置右对齐数据格式，单次转换模式
    ADC1->CR2 &= ~ADC_CR2_ALIGN;    // 右对齐
    ADC1->CR2 &= ~ADC_CR2_CONT;     // 单次转换模式
    ADC1->CR2 &= ~ADC_CR2_EXTSEL;   // 软件触发
    ADC1->CR2 |= ADC_CR2_EXTTRIG;   // 使能外部触发

    finished = 1;
    return;
}

static void adc_power_on( ADC_CHANNEL ch ) {
    // Enable ADC1 Clock
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    // Enbale GPIO Clock
    if ( ch <= ADC_CH7_PA7 ) {
        RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    } else if ( ch <= ADC_CH9_PB1 ) {
        RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    } else if ( ch <= ADC_CH15_PC5 ) {
        RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    }
}
/**
 * @brief 初始化指定ADC通道
 * @param ch ADC通道号 (8 for PB0, 13 for PC3)
 */
uint8_t adc_init(ADC_CHANNEL ch) {

    adc_config_init();
    adc_power_on(ch);

    if ( ch <= ADC_CH9_PB1 ) {
        ADC1->SMPR2 |= ( 0b111 << ( ch * 3 ) );
    } else if ( ch <= ADC_CH15_PC5 ) {
        ADC1->SMPR1 |= ( 0b111 << (( ch - 10 ) * 3) );
    }

    // 校准 ADC
    ADC1->CR2 |= ADC_CR2_ADON;
    for (volatile int i = 0; i < 1000; i++);
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);

    return 0;
}

/**
 * @brief 读取指定ADC通道的单次转换值
 * @param ch ADC通道号
 * @return 12位ADC转换结果 (0-4095)
 */
uint16_t adc_get_single(ADC_CHANNEL ch) {
    // 1. 配置转换序列（即需要转换几个通道）
    ADC1->SQR1 = 0;  // 转换序列长度为1
    ADC1->SQR3 = ch; // 设置第一个转换通道
    
    // 2. 开始转换
    ADC1->CR2 |= ADC_CR2_ADON;  // 启动转换
    
    // 3. 等待转换完成
    while (!(ADC1->SR & ADC_SR_EOC));
    
    // 4. 读取转换结果并清除EOC标志
    uint16_t result = ADC1->DR;
    
    return result;
}
