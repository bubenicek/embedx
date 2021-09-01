
#ifndef __SYSTEM_H
#define __SYSTEM_H

// Application configuration
#include "app_config.h"
#include "compiler.h"

// Platform board
#include "board.h"

// HAL layer
#include "hal_def.h"
#include "cpu/hal_board.h"
#include "cpu/hal_console.h"
#include "cpu/hal_gpio.h"
#include "cpu/hal_led.h"
#include "cpu/hal_spi.h"
#include "cpu/hal_i2c.h"
#include "cpu/hal_uart.h"
#include "cpu/hal_dmx.h"
#include "cpu/hal_time.h"
#include "cpu/hal_pm.h"
#include "cpu/hal_wdg.h"
#include "cpu/hal_uuid.h"
#include "cpu/hal_flash.h"
#include "cpu/hal_rand.h"
#include "cpu/hal_rtc.h"
#include "cpu/hal_net.h"
#include "cpu/hal_adc.h"
#include "cpu/hal_usb.h"
#include "cpu/hal_xmem.h"
#include "cpu/hal_can.h"
#include "cpu/hal_timer.h"

#include "sys/trace.h"
#include "sys/osapi.h"
#include "lib/sysqueue.h"
#include "lib/list.h"
#include "lib/utils.h"


#if defined (CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API == 1)
#include "cmsis_os.h"
#elif defined (CFG_OPENOS_OS_API) && (CFG_OPENOS_OS_API == 1)
#include "openos/scheduler.h"
#include "openos/timer.h"
#endif  

#if defined (CFG_FSLOG_ENABLED) && (CFG_FSLOG_ENABLED == 1)
#include "fslog.h"
#endif

#if defined (CFG_MEMLOG_ENABLED) && (CFG_MEMLOG_ENABLED == 1)
#include "memlog.h"
#endif


#endif // __SYSTEM_H
