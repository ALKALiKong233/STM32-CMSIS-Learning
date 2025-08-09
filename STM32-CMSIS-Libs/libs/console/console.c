#include "console.h"
#include "Driver_Common.h"
#include "Driver_USART.h"
#include <stdint.h>

extern ARM_DRIVER_USART Driver_USART1;
static uint8_t rx_buffer[MAX_CHUNK_SIZE];
static volatile bool rx_complete = false;
static volatile uint32_t rx_count = 0;

void USART1_Event_Callback(uint32_t event) {
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        rx_complete = true;
        rx_count = MAX_CHUNK_SIZE;
        Driver_USART1.Control(ARM_USART_ABORT_RECEIVE, 0);
        Driver_USART1.Receive(rx_buffer, MAX_CHUNK_SIZE);
    }
    
    if (event & ARM_USART_EVENT_RX_TIMEOUT) {
        rx_count = Driver_USART1.GetRxCount();
        rx_complete = true;
        Driver_USART1.Control(ARM_USART_ABORT_RECEIVE, 0);
        Driver_USART1.Receive(rx_buffer, MAX_CHUNK_SIZE);
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
    Driver_USART1.Receive(rx_buffer, MAX_CHUNK_SIZE);
}

uint8_t console_init() {
    USART1_Init();
    return 0;
}

int8_t console_info( uint8_t* msg, uint32_t len ) {
    int8_t res;
    res = Driver_USART1.Send("[INFO]: ", 8);
    if ( res != ARM_DRIVER_OK ) return res;
    while ( Driver_USART1.GetStatus().tx_busy );
    res = Driver_USART1.Send( msg, len );
    if ( res != ARM_DRIVER_OK ) return res;
    while ( Driver_USART1.GetStatus().tx_busy );
    return res;
}

int8_t console_debug( uint8_t* msg, uint32_t len ) {
    int8_t res;
    res = Driver_USART1.Send("[DEBUG]: ", 9);
    if ( res != ARM_DRIVER_OK ) return res;
    while ( Driver_USART1.GetStatus().tx_busy );
    res = Driver_USART1.Send( msg, len );
    if ( res != ARM_DRIVER_OK ) return res;
    while ( Driver_USART1.GetStatus().tx_busy );
    return res;
}

int8_t console_error( uint8_t* msg, uint32_t len ) {
    int8_t res;
    res = Driver_USART1.Send("[ERROR]: ", 9);
    if ( res != ARM_DRIVER_OK ) return res;
    while ( Driver_USART1.GetStatus().tx_busy );
    res = Driver_USART1.Send( msg, len );
    if ( res != ARM_DRIVER_OK ) return res;
    while ( Driver_USART1.GetStatus().tx_busy );
    return res;
}

int8_t console_hex( uint8_t* buf, uint32_t len ) {
    int8_t res;
    res = Driver_USART1.Send(buf, len);
    if ( res != ARM_DRIVER_OK ) return res;
    while ( Driver_USART1.GetStatus().tx_busy );
    return res;
}

int8_t console_read( uint8_t* buf, uint32_t* len ) {
    if ( !rx_complete ) return -1;
    for ( uint32_t i = 0; i < rx_count; ++i ) {
        buf[i] = rx_buffer[i];
    }
    *len = rx_count;
    rx_complete = 0;
    return 0;
}
