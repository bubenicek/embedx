
#ifndef __ESP32_BOARD_H
#define __ESP32_BOARD_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"

#define CFG_HW_VERSION_MODEL            0
#define CFG_HW_VERSION_REVISION         1


/** Application entry point function */
#define APPLICATION_MAIN()  void app_main(void)

/** Trace float formating */
#define CFG_TRACE_HAS_CVTBUF_FUNCTIONS          1

#define CFG_ESP32_HEAP_SIZE                     (256 * 1024)

/** FreeRTOS missing defines */
#define portEND_SWITCHING_ISR( xSwitchRequired ) if( xSwitchRequired != pdFALSE ) portYIELD()
static inline size_t xPortGetTotalHeapSize(void)
{
   return CFG_ESP32_HEAP_SIZE;
}

/** GPIO configuration */
#define CFG_HAL_GPIO_DEF {    \
   {2, HAL_GPIO_MODE_OUTPUT, 1},     \
}

/** LED configuration */
#define CFG_HAL_LED_DEF { \
   HAL_GPIO0, 1,          \
}

/** UART configuration */
#define CFG_HAL_UART_DEF { \
   {UART_NUM_1, 115200, GPIO_NUM_16, GPIO_NUM_17}, \
}

//
// Board configuration
//
#define LED_SYSTEM                              HAL_LED0
#define LED_ERROR                               HAL_LED0


#endif   // __ESP32_BOARD_H
