#include "dht11.h"
#include "../delay/delay.h"
#include <stdint.h>

static uint8_t save_data_from_buf( uint8_t *buf, dht11_dt *dest ) {
    dest->negative = ((1<<7) & buf[3]) ? 1 : 0;
    dest->temp = buf[2];
    dest->temp_dec = ~(1<<7) & buf[3];
    dest->humity = buf[0];
    dest->humity_dec = buf[1];
    dest->check = buf[4];
    return 0;
}

// Reset DHT11.
void dht11_rst(void)
{
    DHT11_IO_OUT(); // SET OUTPUT
    DHT11_DQ_OUT(0); // 拉低 DQ
    delay_ms(20);    // 拉低至少18ms
    DHT11_DQ_OUT(1); // 拉高 DQ
    delay_us(20);    // 主机拉高20~40us
}

// Check the connectivity between DHT11 and MCU.
uint8_t dht11_check(void)
{
    uint16_t retry = 0;
    DHT11_IO_IN(); // SET INPUT
    
    // 等待DHT11拉低（响应信号开始）
    while (DHT11_DQ_IN && retry < 1000)
    {
        retry++;
        delay_us(1);
    }
    if(retry >= 1000) return 1; // 超时，DHT11没有响应
    
    retry = 0;
    // 等待DHT11拉高（响应信号结束）
    while (!DHT11_DQ_IN && retry < 1000) 
    {
        retry++;
        delay_us(1);
    }
    if(retry >= 1000) return 1; // 超时
    
    return 0;
}

static uint8_t dht11_read_bit(void)
{
    uint16_t retry = 0;
    
    // 等待变为低电平（每个位开始）
    while(DHT11_DQ_IN && retry < 100) 
    {
        retry++;
        delay_us(1);
    }
    if(retry >= 100) return 0; // 超时，返回0
    
    retry = 0;
    // 等待变高电平（数据位）
    while(!DHT11_DQ_IN && retry < 100) 
    {
        retry++;
        delay_us(1);
    }
    if(retry >= 100) return 0; // 超时，返回0
    
    // Bit 0 -> 23-27us 高电平
    // Bit 1 -> 68-74us 高电平
    // 如果高电平持续时间较长，则为'1'，否则为'0'
    // 延时50ms后检测当前为高电平还是低电平
    // 不能使用 delay_us(1) 来计算持续了多少微秒
    // 因为在 1us 时间内 delay_us 调用的时间就已经远大于 1us
    delay_us(50);
    if(DHT11_DQ_IN) return 1;
    else return 0;
}

static uint8_t dht11_read_byte(void)
{
    uint8_t i, dat;
    dat = 0;
    
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= dht11_read_bit();
    }
    
    return dat;
}

// Read datas from DHT11.
uint8_t dht11_read(dht11_dt *datavalue)
{
    uint8_t buf[5];
    uint8_t i;
    
    dht11_rst();
    
    uint8_t check_result = dht11_check();
    if(check_result == 0)
    {
        for(i = 0; i < 5; i++) 
        {
            buf[i] = dht11_read_byte();
        }
        
        uint8_t checksum = buf[0] + buf[1] + buf[2] + buf[3];
        
        if ( checksum == buf[4] ) {
            save_data_from_buf(buf, datavalue);
        }
        
        // 0 -> Success, 2-> Checksum falied.
        return ( checksum == buf[4] ) ? 0 : 2;
    }
    else
    {
        return 1; // Failed to communicate with DHT11.
    }
}

// Init DHT11
uint8_t dht11_init(void)
{
    // 使能 DHT11 所在 Pin 的时钟
    RCC->APB2ENR |= DHT11_GPIO_CLK;
    
    // 配置GPIO为推挽输出
    DHT11_IO_OUT();
    DHT11_DQ_OUT(1);
    
    // DHT11需要上电后至少1秒才能稳定
    delay_ms(1500);  // 等待1.5秒确保稳定
    
    dht11_rst();
    
    return dht11_check();
}
