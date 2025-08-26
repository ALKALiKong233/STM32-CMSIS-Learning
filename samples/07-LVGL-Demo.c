#include "RTE_Components.h"
#include "console/console.h"
#include "core/lv_obj.h"
#include "widgets/lv_label.h"
#include <stdint.h>
#include CMSIS_device_header
#include "libs/console/console.h"
#include "libs/delay/delay.h"
#include "libs/dht11/dht11.h"
#include "interface/adc/adc.h"
#include "lv_port_disp.h"

// Widgets
static lv_obj_t * slider_temperature;
static lv_obj_t * slider_humidity;
static lv_obj_t * label_temp_value;
static lv_obj_t * label_humi_value;
static lv_obj_t * bar_adc_pb0;
static lv_obj_t * bar_adc_pc3;
static lv_obj_t * label_adc_pb0;
static lv_obj_t * label_adc_pc3;

// Sensor data
static dht11_dt dht11_data;
static uint16_t adc_pb0_value = 0;
static uint16_t adc_pc3_value = 0;

static void update_sensor_data()
{
    // DHT11 Sensor
    static uint32_t last_dht11_read = 0;
    uint32_t current_tick = delay_get_tick();
    
    if(timer_expired(&last_dht11_read, 2000, current_tick)) {
        uint8_t result = dht11_read(&dht11_data);
        if(result == 0) {
            int temp_range = dht11_data.temp > 50 ? 50 : dht11_data.temp;
            lv_slider_set_value(slider_temperature, temp_range, LV_ANIM_ON);
            lv_label_set_text_fmt(label_temp_value, "%d°C", dht11_data.temp);
            
            lv_slider_set_value(slider_humidity, dht11_data.humity, LV_ANIM_ON);
            lv_label_set_text_fmt(label_humi_value, "%d%%", dht11_data.humity);
        }
    }
    
    // ADC Sensors
    static uint32_t last_adc_read = 0;
    if(timer_expired(&last_adc_read, 100, current_tick)) {
        // PB0
        adc_pb0_value = adc_get_single(ADC_CH8_PB0);
        uint8_t pb0_percent = (adc_pb0_value * 100) / 4095;
        lv_bar_set_value(bar_adc_pb0, pb0_percent, LV_ANIM_ON);
        lv_label_set_text_fmt(label_adc_pb0, "%d%% (%d)", pb0_percent, adc_pb0_value);
        
        // PC3
        adc_pc3_value = adc_get_single(ADC_CH13_PC3);
        uint8_t pc3_percent = (adc_pc3_value * 100) / 4095;
        lv_bar_set_value(bar_adc_pc3, pc3_percent, LV_ANIM_ON);
        lv_label_set_text_fmt(label_adc_pc3, "%d%% (%d)", pc3_percent, adc_pc3_value);
    }
}

// User Interfaces
static void create_ui(void)
{
    // 1. Title label
    lv_obj_t * title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "LiTown DingZhen LVGL");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    // 2. Temperature Region
    lv_obj_t * temp_label = lv_label_create(lv_scr_act());
    lv_label_set_text(temp_label, "TEMP (0-50°C)");
    lv_obj_align(temp_label, LV_ALIGN_TOP_LEFT, 10, 30);
    
    slider_temperature = lv_slider_create(lv_scr_act());
    lv_obj_set_size(slider_temperature, 180, 20);
    lv_obj_align(slider_temperature, LV_ALIGN_TOP_LEFT, 10, 50);
    lv_slider_set_range(slider_temperature, 0, 50);
    lv_slider_set_value(slider_temperature, 25, LV_ANIM_OFF);
    lv_obj_clear_flag(slider_temperature, LV_OBJ_FLAG_CLICKABLE); // 只读显示
    
    label_temp_value = lv_label_create(lv_scr_act());
    lv_label_set_text(label_temp_value, "TEMP: --°C");
    lv_obj_align_to(label_temp_value, slider_temperature, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // 3. Humidity Region
    lv_obj_t * humi_label = lv_label_create(lv_scr_act());
    lv_label_set_text(humi_label, "HUMI (0-100%)");
    lv_obj_align(humi_label, LV_ALIGN_TOP_LEFT, 10, 80);
    
    slider_humidity = lv_slider_create(lv_scr_act());
    lv_obj_set_size(slider_humidity, 180, 20);
    lv_obj_align(slider_humidity, LV_ALIGN_TOP_LEFT, 10, 100);
    lv_slider_set_range(slider_humidity, 0, 100);
    lv_slider_set_value(slider_humidity, 50, LV_ANIM_OFF);
    lv_obj_clear_flag(slider_humidity, LV_OBJ_FLAG_CLICKABLE); // 只读显示
    
    label_humi_value = lv_label_create(lv_scr_act());
    lv_label_set_text(label_humi_value, "HUMI: --%");
    lv_obj_align_to(label_humi_value, slider_humidity, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // 4. ADC PB0 Region
    lv_obj_t * pb0_label = lv_label_create(lv_scr_act());
    lv_label_set_text(pb0_label, "PB0 ADC (0-100%)");
    lv_obj_align(pb0_label, LV_ALIGN_TOP_LEFT, 10, 130);
    
    bar_adc_pb0 = lv_bar_create(lv_scr_act());
    lv_obj_set_size(bar_adc_pb0, 180, 20);
    lv_obj_align(bar_adc_pb0, LV_ALIGN_TOP_LEFT, 10, 150);
    lv_bar_set_value(bar_adc_pb0, 0, LV_ANIM_OFF);
    
    label_adc_pb0 = lv_label_create(lv_scr_act());
    lv_label_set_text(label_adc_pb0, "PB0: 0% (0)");
    lv_obj_align_to(label_adc_pb0, bar_adc_pb0, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // 5. ADC PC3 Region
    lv_obj_t * pc3_label = lv_label_create(lv_scr_act());
    lv_label_set_text(pc3_label, "PC3 ADC (0-100%)");
    lv_obj_align(pc3_label, LV_ALIGN_TOP_LEFT, 10, 180);
    
    bar_adc_pc3 = lv_bar_create(lv_scr_act());
    lv_obj_set_size(bar_adc_pc3, 180, 20);
    lv_obj_align(bar_adc_pc3, LV_ALIGN_TOP_LEFT, 10, 200);
    lv_bar_set_value(bar_adc_pc3, 0, LV_ANIM_OFF);
    
    label_adc_pc3 = lv_label_create(lv_scr_act());
    lv_label_set_text(label_adc_pc3, "PC3: 0% (0)");
    lv_obj_align_to(label_adc_pc3, bar_adc_pc3, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // 6. LED Indicator
    lv_obj_t * led = lv_led_create(lv_scr_act());
    lv_obj_set_size(led, 30, 30);
    lv_obj_align(led, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_led_on(led);
    
    lv_obj_t * led_label = lv_label_create(lv_scr_act());
    lv_label_set_text(led_label, "System Running");
    lv_obj_align_to(led_label, led, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
}

int main() {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;

    delay_init();
    console_init();
    
    console_info((uint8_t*)"System starting...\n", 20);

    dht11_init();
    console_info((uint8_t*)"DHT11 initialized\r\n", 19);

    adc_init(ADC_CH8_PB0);   // PB0
    adc_init(ADC_CH13_PC3);  // PC3
    console_info((uint8_t*)"ADC channels initialized\r\n", 27);

    // Initialize LVGL
    lv_init();
    lv_port_disp_init();
    console_info((uint8_t*)"LVGL initialized\r\n", 19);

    // Create UI
    create_ui();
    console_info((uint8_t*)"UI created\r\n", 13);

    console_info((uint8_t*)"Starting main loop...\r\n", 24);


    for (;;) {
        static uint32_t last_lv_timer = 0;
        uint32_t current_tick = delay_get_tick();
        if ( timer_expired(&last_lv_timer, 5, current_tick))
            lv_timer_handler();

        update_sensor_data();
    }
}
