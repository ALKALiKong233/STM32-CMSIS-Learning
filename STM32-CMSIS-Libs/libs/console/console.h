#ifndef LIBS_CONSOLE_H
#define LIBS_CONSOLE_H

#include <stdint.h>
#include "libs_common.h"

#define MAX_CHUNK_SIZE 2048

#define CONSOLE_READ_OK 0
#define CONSOLE_READ_NO_NEW_MSG -1

uint8_t console_init();

int8_t console_info( uint8_t* msg, uint32_t len );
int8_t console_debug( uint8_t* msg, uint32_t len );
int8_t console_error( uint8_t* msg, uint32_t len );
int8_t console_hex( uint8_t* buf, uint32_t len );

// Return 0 if a new message is received since last read, -1 if no new msg.
int8_t console_read( uint8_t* buf, uint32_t* len );

#endif