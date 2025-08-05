#include "Driver_Common.h"
#include "Driver_USART.h"
#include "RTE_Components.h"
#include "stdint.h"
#include CMSIS_device_header
#include "string.h"
#include "stdio.h"
#include "libs/st7789/simple_st7789_driver.h"
#include "libs/delay/delay.h"
#include "libs/dht11/dht11.h"

extern ARM_DRIVER_USART Driver_USART1;

void USART1_Event_Callback(uint32_t event) {
}

void USART1_Init() {
    Driver_USART1.Initialize(USART1_Event_Callback);
    Driver_USART1.PowerControl(ARM_POWER_FULL);
    Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
                            ARM_USART_DATA_BITS_8 |
                            ARM_USART_PARITY_NONE |
                            ARM_USART_STOP_BITS_1 |
                            ARM_USART_FLOW_CONTROL_NONE, 115200);
    Driver_USART1.Control(ARM_USART_CONTROL_TX, 1);
    Driver_USART1.Control(ARM_USART_CONTROL_RX, 1);
}

int main() {
    // 使能GPIO时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;

    // 初始化延时系统
    delay_init();
    
    // 初始化USART
    USART1_Init();

    // 发送启动消息
    const char* welcome = "\n=== DHT11 Temperature & Humidity Sensor Demo ===\n";
    Driver_USART1.Send(welcome, strlen(welcome));
    while (Driver_USART1.GetStatus().tx_busy);

    // 初始化显示屏
    simple_st7789_init();
    simple_st7789_fill_screen(COLOR_BLUE);
    simple_st7789_draw_string(10, 10, "DHT11 Sensor", COLOR_WHITE, COLOR_BLUE);
    simple_st7789_draw_string(10, 40, "Demo", COLOR_WHITE, COLOR_BLUE);
    
    // 初始化DHT11传感器
    const char* init_msg = "Initializing DHT11...\n";
    Driver_USART1.Send(init_msg, strlen(init_msg));
    while (Driver_USART1.GetStatus().tx_busy);
    
    simple_st7789_draw_string(10, 70, "Initializing...", COLOR_YELLOW, COLOR_BLUE);
    
    while(dht11_init() != 0) {
        const char* error_msg = "DHT11 initialization failed! Check wiring.\n";
        Driver_USART1.Send(error_msg, strlen(error_msg));
        while (Driver_USART1.GetStatus().tx_busy);
        
        simple_st7789_draw_string(10, 100, "Init Failed!", COLOR_RED, COLOR_BLUE);
        simple_st7789_draw_string(10, 130, "Check Wiring", COLOR_RED, COLOR_BLUE);
        
        delay_ms(2000);
    }
    
    const char* init_ok = "DHT11 initialized successfully!\n\n";
    Driver_USART1.Send(init_ok, strlen(init_ok));
    while (Driver_USART1.GetStatus().tx_busy);
    
    simple_st7789_draw_string(10, 70, "Init Success! ", COLOR_GREEN, COLOR_BLUE);
    simple_st7789_draw_string(10, 100, "             ", COLOR_BLUE, COLOR_BLUE); // 清除错误信息
    simple_st7789_draw_string(10, 130, "             ", COLOR_BLUE, COLOR_BLUE);

    dht11_dt sensor_data;
    char temp_str[50];
    char humi_str[50];
    char uart_msg[100];
    uint32_t read_count = 0;
    
    uint32_t time_expr = 0;
    for (;;) {
        if ( )
        read_count++;
        
        // 发送读取计数
        sprintf(uart_msg, "Reading #%d:\n", (int)read_count);
        Driver_USART1.Send(uart_msg, strlen(uart_msg));
        while (Driver_USART1.GetStatus().tx_busy);
        
        // 读取DHT11数据
        if(dht11_read(&sensor_data) == 0) {
            // 读取成功
            sprintf(temp_str, "Temperature: %d°C", sensor_data.temp);
            sprintf(humi_str, "Humidity: %d%%RH", sensor_data.humity);
            
            // 通过USART输出详细数据
            sprintf(uart_msg, "  %s\n", temp_str);
            Driver_USART1.Send(uart_msg, strlen(uart_msg));
            while (Driver_USART1.GetStatus().tx_busy);
            
            sprintf(uart_msg, "  %s\n", humi_str);
            Driver_USART1.Send(uart_msg, strlen(uart_msg));
            while (Driver_USART1.GetStatus().tx_busy);
            
            sprintf(uart_msg, "  Checksum: 0x%02X (Valid)\n\n", sensor_data.check);
            Driver_USART1.Send(uart_msg, strlen(uart_msg));
            while (Driver_USART1.GetStatus().tx_busy);
            
            // 在显示屏上显示数据
            simple_st7789_draw_string(10, 100, temp_str, COLOR_WHITE, COLOR_BLUE);
            simple_st7789_draw_string(10, 130, humi_str, COLOR_WHITE, COLOR_BLUE);
            
            // 显示读取次数
            sprintf(uart_msg, "Count: %d", (int)read_count);
            simple_st7789_draw_string(10, 160, uart_msg, COLOR_CYAN, COLOR_BLUE);
            
            // 显示校验和
            sprintf(uart_msg, "CRC: 0x%02X", sensor_data.check);
            simple_st7789_draw_string(10, 190, uart_msg, COLOR_YELLOW, COLOR_BLUE);
            
        } else {
            // 读取失败
            const char* error_msg = "  DHT11 read failed! (Timeout or CRC error)\n\n";
            Driver_USART1.Send(error_msg, strlen(error_msg));
            while (Driver_USART1.GetStatus().tx_busy);
            
            simple_st7789_draw_string(10, 100, "Read Failed!   ", COLOR_RED, COLOR_BLUE);
            simple_st7789_draw_string(10, 130, "Check Sensor   ", COLOR_RED, COLOR_BLUE);
        }
        
        // 等待2秒再次读取
        // DHT11建议读取间隔不少于100ms，这里用2秒保证稳定性
        delay_ms(2000);
    }
}
