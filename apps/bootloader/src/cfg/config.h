
#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_HW_VERSION_MODEL        1
#define CFG_HW_VERSION_REVISION     0

#define CFG_FW_VERSION_MAJOR        1
#define CFG_FW_VERSION_MINOR        0

//-----------------------------------------------------------------------------
//                      System configuration
//-----------------------------------------------------------------------------
#define CFG_DEBUG                            1
#define CFG_DEBUG_TIMESTAMP                  1
#define CFG_CMSIS_OS_API                     0
#define CFG_HAL_ETH_ENABLED                  0
#define CFG_HAL_CONSOLE_BUFFERED             0
#define CFG_HAL_UART_ENABLED                 0
#define CFG_HAL_WDG_ENABLED                  1

//-----------------------------------------------------------------------------
//                      Debug trace configuration
//-----------------------------------------------------------------------------
#define CFG_ENABLE_TRACE                     1
#define ENABLE_TRACE_HAL                     1
#define ENABLE_TRACE_MAIN                    1
#define ENABLE_TRACE_DFS                     1


#endif   // __CONFIG_H
