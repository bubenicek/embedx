
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "system.h"
#include "bootloader/dfs.h"

#define TRACE_TAG "Bootloader"
#if !ENABLE_TRACE_MAIN
#undef TRACE
#define TRACE(...)
#endif

// Application header
__attribute__((section(".app_header"))) const app_header_t app_header =
{
   .magic = APP_HEADER_MAGIC,
   .hw_version = APP_VERSION(CFG_HW_VERSION_MODEL, CFG_HW_VERSION_REVISION),
   .fw_version = APP_VERSION(CFG_FW_VERSION_MAJOR, CFG_FW_VERSION_MINOR),
   .fw_size = 0,
   .fw_crc = 0,
};

/** pointer to function, used for jump to main application */
typedef void (*jump_function_t)(void);

static void jump_app(void)
{
   uint32_t jump_address;
   jump_function_t jump_to_application;

   // Jump to user application
   jump_address = *(__IO uint32_t *) (CFG_DFS_ACTIVE_START_ADDR + 4);
   jump_to_application = (jump_function_t)jump_address;

   TRACE("Stack ptr = 0x%X", *(__IO uint32_t *) CFG_DFS_ACTIVE_START_ADDR);
   TRACE("Jump_address = 0x%X", jump_address);
   TRACE("Running application");

   __set_PRIMASK(1);
    RCC_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

   // Initialize user application's Stack Pointer
   __set_MSP(*(__IO uint32_t *) CFG_DFS_ACTIVE_START_ADDR);

   // run user application
   jump_to_application();
}

int main(void)
{
   app_header_t hdr;

   // Initialize HW board
   VERIFY_FATAL(board_init() == 0);

   // Initialize DFS
   VERIFY_FATAL(dfs_init() == 0);

   TRACE("Bootloader ver. %d.%d  ", CFG_FW_VERSION_MAJOR, CFG_FW_VERSION_MINOR);



   if (dfs_open(DFS_UPGRADE, &hdr) == 0)
   {
      //
      // Upgrade application
      //
      dfs_close(DFS_UPGRADE);

      hal_led_set(LED_DF, 1);
      hal_led_set(LED_ZWAVE, 1);
      hal_led_set(LED_SYSTEM, 1);

      // Upgrade app
      TRACE("Upgrade FW ver: %d.%d  size: %d", APP_VERSION_MAJOR(hdr.fw_version), APP_VERSION_MINOR(hdr.fw_version), hdr.fw_size);

      // Copy app from upgrade to active dfs
      if (dfs_copy(DFS_UPGRADE, DFS_ACTIVE) == 0)
         TRACE("Upgrade done");
      else
         TRACE("Upgrade failed");

      // Remove upgrade file
      dfs_erase(DFS_UPGRADE);
      dfs_deinit();

      hal_reset();
   }
   else if (dfs_open(DFS_ACTIVE, &hdr) == 0)
   {
      //
      // Run application
      //
      dfs_close(DFS_ACTIVE);
      dfs_deinit();

      // Run application
      jump_app();
   }

   TRACE_ERROR("Bootloader failed");
   dfs_deinit();

   // Inifinite error loop
   while(1)
   {
      hal_led_toggle(LED_DF);
      hal_led_toggle(LED_ZWAVE);
      hal_led_toggle(LED_SYSTEM);

      hal_wdg_reset();
      hal_delay_ms(50);
   }

   return 0;
}
