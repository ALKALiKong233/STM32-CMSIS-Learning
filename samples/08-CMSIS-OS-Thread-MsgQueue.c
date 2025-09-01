#include "RTE_Components.h"
#include "cmsis_os2.h"
#include <stdint.h>
#include CMSIS_device_header
#include "st7789/simple_st7789_driver.h"
#include "delay/delay.h"
#include "led/led_gpio.h"
#include "dht11/dht11.h"
#include "stdio.h"

uint8_t DHT11_Status;

struct LED_INFO {
    char bank;
    uint8_t pin;
} led_list[4] = { {'D', 12}, {'D', 11},
                    {'D', 9}, {'B', 8} };

osThreadId_t LED1_Thread_Handle;
const osThreadAttr_t LED1Task_attributes = {
   .name = "LED1Task",
   .priority = (osPriority_t) osPriorityNormal,
   .stack_size = 128 * 4
};

osThreadId_t LED2_Thread_Handle;
const osThreadAttr_t LED2Task_attributes = {
   .name = "LED2Task",
   .priority = (osPriority_t) osPriorityNormal,
   .stack_size = 128 * 4
};

osThreadId_t DHT11_READ_Thread_Handle;
const osThreadAttr_t DHT11_READ_Task_attributes = {
   .name = "DHT11 Read Task",
   .priority = (osPriority_t) osPriorityNormal,
   .stack_size = 128 * 4
};

osThreadId_t DHT11_DISPLAY_Thread_Handle;
const osThreadAttr_t DHT11_DISPLAY_Task_attributes = {
   .name = "DHT11 Display Task",
   .priority = (osPriority_t) osPriorityNormal,
   .stack_size = 128 * 4
};

void LED1_Task() {
    for(;;) {
        led_on(led_list[0].bank, led_list[0].pin);
        delay_ms(100);
        led_off(led_list[0].bank, led_list[0].pin);
        delay_ms(500);
    }
}

void LED2_Task() {
    for(;;) {
        led_on(led_list[1].bank, led_list[1].pin);
        delay_ms(100);
        led_off(led_list[1].bank, led_list[1].pin);
        delay_ms(100);
    }
}

osMessageQueueId_t DHT11_MSG_Handle = NULL;

void DHT11_Read_Task() {
    DHT11_Status = dht11_init();
    if (DHT11_Status == 0) {
        simple_st7789_draw_string(5, 21, "DHT11 Inited.", COLOR_BLACK, COLOR_WHITE);
    } else {
        simple_st7789_draw_string(5, 21, "DHT11 Failed.", COLOR_BLACK, COLOR_WHITE);
    }
    DHT11_MSG_Handle = osMessageQueueNew(4, sizeof(dht11_dt), NULL);
    dht11_dt data;
    for(;;) {
        uint8_t res = dht11_read(&data);
        if ( res == 0 ) {
            osMessageQueuePut(DHT11_MSG_Handle, &data, NULL, osWaitForever);
        }
        delay_ms(2000);
    }
}

void DHT11_Display_Task() {
    char temp_str[50];
    char humi_str[50];
    char time_str[50];
    dht11_dt data;
    uint32_t time;
    while ( DHT11_MSG_Handle == NULL );
    for(;;) {
        osMessageQueueGet(DHT11_MSG_Handle, &data, NULL, osWaitForever);
        time = delay_get_tick();
        sprintf(temp_str, "Temperature: %d°C", data.temp);
        sprintf(humi_str, "Humidity: %d%%RH", data.humity);
        sprintf(time_str, "Time: %d", time);
        simple_st7789_draw_string(5, 100, temp_str, COLOR_BLACK, COLOR_WHITE);
        simple_st7789_draw_string(5, 130, humi_str, COLOR_BLACK, COLOR_WHITE);
        simple_st7789_draw_string(5, 160, time_str, COLOR_BLACK, COLOR_WHITE);
    }
}

void rtos_tasks_init() {
    LED1_Thread_Handle = osThreadNew( LED1_Task, NULL, &LED1Task_attributes );
    LED2_Thread_Handle = osThreadNew( LED2_Task, NULL, &LED2Task_attributes );
    DHT11_READ_Thread_Handle = osThreadNew(DHT11_Read_Task, NULL, &DHT11_READ_Task_attributes);
    DHT11_DISPLAY_Thread_Handle = osThreadNew(DHT11_Display_Task, NULL, &DHT11_DISPLAY_Task_attributes);
}

int main() {
    osKernelInitialize();

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;

    delay_init();
    simple_st7789_init();
    simple_st7789_fill_screen(COLOR_WHITE);

    for ( uint8_t i = 0; i < 4; ++i ) {
        led_init(led_list[i].bank, led_list[i].pin);
    }

    rtos_tasks_init();

    osKernelStart();

    for(;;) {

    }
}
