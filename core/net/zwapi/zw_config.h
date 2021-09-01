
#ifndef __ZW_CONFIG_H
#define __ZW_CONFIG_H

#include "system.h"

//
// Traces options
//
#ifndef ENABLE_TRACE_ZW_SERIALAPI
#define ENABLE_TRACE_ZW_SERIALAPI          0
#endif

#ifndef ENABLE_TRACE_ZW_SERIALAPI_LINK
#define ENABLE_TRACE_ZW_SERIALAPI_LINK     0
#endif

#ifndef ENABLE_TRACE_ZW_UART
#define ENABLE_TRACE_ZW_UART               0
#endif


//
// Threads configuration
//
#ifndef ZW_SERIALAPI_LINK_THREAD_PRIORITY
#define ZW_SERIALAPI_LINK_THREAD_PRIORITY      osPriorityNormal
#endif
#ifndef ZW_SERIALAPI_LINK_THREAD_STACK_SIZE
#define ZW_SERIALAPI_LINK_THREAD_STACK_SIZE    512
#endif

#ifndef ZW_SERIALAPI_THREAD_PRIORITY
#define ZW_SERIALAPI_THREAD_PRIORITY           osPriorityNormal
#endif
#ifndef ZW_SERIALAPI_THREAD_STACK_SIZE
#define ZW_SERIALAPI_THREAD_STACK_SIZE         512
#endif


//
// External eeproms configuration
//
#define ZW_EEPROM_TYPE_M95256                   1
#define ZW_EEPROM_TYPE_M25PE10                  2   

#ifndef ZW_EEPROM_TYPE
#define ZW_EEPROM_TYPE                          ZW_EEPROM_TYPE_M25PE10
#endif


//
// Uart interface
//
#define ZW_UART_BAUDRATE            115200
#define zw_uart_open()              hal_uart_init(UART_ZW, ZW_UART_BAUDRATE)
#define zw_uart_close()             hal_uart_deinit(UART_ZW)
#define zw_uart_putchar(_c)         hal_uart_putchar(UART_ZW, _c)
#define zw_uart_getchar(_timeout)   hal_uart_getchar(UART_ZW, _timeout)
#define zw_uart_flush()             hal_uart_flush(UART_ZW)


#endif // __ZW_CONFIG_H
