/**
 * Watchdog is configured with 4 seconds timeout
 * Based on IWDG/IWDG_Example/main.c (V1.0.1, 13-April-2012), sample file for IWDG periphery from STM32F4xx_DSP_StdPeriph_Lib_V1.0.2
 * @note Currently only internal watchdog is enabled
 */

#include "system.h"

#if CFG_HAL_WDG_ENABLED

typedef enum
{
   TM_WATCHDOG_Timeout_5ms = 0x00,   /*!< System reset called every 5ms */
   TM_WATCHDOG_Timeout_10ms = 0x01,  /*!< System reset called every 10ms */
   TM_WATCHDOG_Timeout_15ms = 0x02,  /*!< System reset called every 15ms */
   TM_WATCHDOG_Timeout_30ms = 0x03,  /*!< System reset called every 30ms */
   TM_WATCHDOG_Timeout_60ms = 0x04,  /*!< System reset called every 60ms */
   TM_WATCHDOG_Timeout_120ms = 0x05, /*!< System reset called every 120ms */
   TM_WATCHDOG_Timeout_250ms = 0x06, /*!< System reset called every 250ms */
   TM_WATCHDOG_Timeout_500ms = 0x07, /*!< System reset called every 500ms */
   TM_WATCHDOG_Timeout_1s = 0x08,    /*!< System reset called every 1s */
   TM_WATCHDOG_Timeout_2s = 0x09,    /*!< System reset called every 2s */
   TM_WATCHDOG_Timeout_4s = 0x0A,    /*!< System reset called every 4s */
   TM_WATCHDOG_Timeout_8s = 0x0B,    /*!< System reset called every 8s */
   TM_WATCHDOG_Timeout_16s = 0x0C,   /*!< System reset called every 16s */
   TM_WATCHDOG_Timeout_32s = 0x0D    /*!< System reset called every 32s. This is maximum value allowed with IWDG timer */

} TM_WATCHDOG_Timeout_t;

#ifndef CFG_HAL_WDG_TIMEOUT
#define CFG_HAL_WDG_TIMEOUT    TM_WATCHDOG_Timeout_8s
#endif


int hal_wdg_init(void)
{
   uint16_t reload = 0;
   TM_WATCHDOG_Timeout_t timeout = CFG_HAL_WDG_TIMEOUT;  

    __HAL_RCC_WWDG_CLK_ENABLE();

   /* Enable write access to IWDG_PR and IWDG_RLR registers */
   IWDG->KR = 0x5555;

   /* Set proper clock depending on timeout user select */
   if (timeout >= TM_WATCHDOG_Timeout_8s)
   {
      /* IWDG counter clock: LSI/256 = 128Hz */
      IWDG->PR = 0x07;
   }
   else
   {
      /* IWDG counter clock: LSI/32 = 1024Hz */
      IWDG->PR = 0x03;
   }

   /* Set counter reload value */
   if (timeout == TM_WATCHDOG_Timeout_5ms)
   {
      reload = 5; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_10ms)
   {
      reload = 10; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_15ms)
   {
      reload = 15; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_30ms)
   {
      reload = 31; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_60ms)
   {
      reload = 61; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_120ms)
   {
      reload = 123; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_250ms)
   {
      reload = 255; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_500ms)
   {
      reload = 511; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_1s)
   {
      reload = 1023; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_2s)
   {
      reload = 2047; /* 1024 Hz IWDG ticking */
      reload = 2047; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_4s)
   {
      reload = 4095; /* 1024 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_8s)
   {
      reload = 1023; /* 128 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_16s)
   {
      reload = 2047; /* 128 Hz IWDG ticking */
   }
   else if (timeout == TM_WATCHDOG_Timeout_32s)
   {
      reload = 4095; /* 128 Hz IWDG ticking */
   }

   /* Set reload */
   IWDG->RLR = reload;

   /* Reload IWDG counter */
   IWDG->KR = 0xAAAA;

   /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
   IWDG->KR = 0xCCCC;

   return 0;
}

void hal_wdg_reset(void)
{
  /* Reload IWDG counter */
  IWDG->KR = 0xAAAA;
}

uint8_t hal_wdg_reset_occured(void)
{
   return 0; 
}

#endif   // CFG_HAL_WDG_ENABLED






