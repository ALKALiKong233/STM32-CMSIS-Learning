#include "Driver_Common.h"
#include "Driver_USART.h"
#include "RTE_Components.h"
#include "stdint.h"
#include CMSIS_device_header
#include "string.h"

extern ARM_DRIVER_USART Driver_USART1;
static uint8_t rx_buffer[256];
static volatile bool rx_complete = false;
static volatile uint32_t rx_count = 0;

void USART1_Event_Callback(uint32_t event) {
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        // 接收到完整的256字节
        rx_complete = true;
        rx_count = 256;
        // 停止接收
        Driver_USART1.Control(ARM_USART_ABORT_RECEIVE, 0);
    }
    
    if (event & ARM_USART_EVENT_RX_TIMEOUT) {
        // IDLE检测到，获取实际接收的字节数
        rx_count = Driver_USART1.GetRxCount();
        rx_complete = true;
        // 停止接收
        Driver_USART1.Control(ARM_USART_ABORT_RECEIVE, 0);
    }
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
    USART1_Init();
    // 发送启动消息
    const char* welcome = "USART CMSIS Demo Ready! Send data (max 256 bytes):\r\n";
    Driver_USART1.Send(welcome, strlen(welcome));
    while (Driver_USART1.GetStatus().tx_busy);

    for (;;) {
        // 清空缓冲区
        memset(rx_buffer, 0, sizeof(rx_buffer));
        
        // 启动接收256字节
        rx_complete = false;
        rx_count = 0;
        int32_t result = Driver_USART1.Receive(rx_buffer, 256);
        
        if (result == ARM_DRIVER_OK) {
            // 简陋的计数
            uint32_t timeout_counter = 0;
            const uint32_t MAX_TIMEOUT = 100000;
            
            // 等待接收完成
            while (!rx_complete && timeout_counter < MAX_TIMEOUT) {
                // 简单延时，让系统有时间处理中断
                for (volatile int i = 0; i < 100; i++) __NOP();
                timeout_counter++;
            }
            
            if (timeout_counter >= MAX_TIMEOUT) {
                // 超时处理
                const char* timeout_msg = "Timeout! No data received.\r\n";
                Driver_USART1.Send(timeout_msg, strlen(timeout_msg));
                while (Driver_USART1.GetStatus().tx_busy);
                
                // 取消当前接收
                Driver_USART1.Control(ARM_USART_ABORT_RECEIVE, 0);
            } else {
                // 正常接收到数据
                if (rx_count > 0) {
                    // 发送确认消息
                    const char* ack = "Received: ";
                    Driver_USART1.Send(ack, strlen(ack));
                    while (Driver_USART1.GetStatus().tx_busy);
                    
                    // 回显接收到的数据
                    Driver_USART1.Send(rx_buffer, rx_count);
                    while (Driver_USART1.GetStatus().tx_busy);
                    
                    // 换行
                    const char* newline = "\r\n";
                    Driver_USART1.Send(newline, 2);
                    while (Driver_USART1.GetStatus().tx_busy);
                } else {
                    const char* no_data = "No data received.\r\n";
                    Driver_USART1.Send(no_data, strlen(no_data));
                    while (Driver_USART1.GetStatus().tx_busy);
                }
                Driver_USART1.Control(ARM_USART_ABORT_RECEIVE, 0);
            }
        } else {
            // 接收启动失败
            const char* error_msg = "Failed to start receive!\r\n";
            Driver_USART1.Send(error_msg, strlen(error_msg));
            while (Driver_USART1.GetStatus().tx_busy);
        }
        
        // 短暂延时，然后继续下一轮接收
        for (volatile uint32_t i = 0; i < 100000; i++) __NOP();
    }
}
