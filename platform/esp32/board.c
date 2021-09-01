
#include "system.h"

#define TRACE_TAG		"hal"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif


int board_init(void)
{   
   int res = 0;
   esp_err_t ret;
   esp_chip_info_t chip_info;

   // Initialize HAL drivers
   res += trace_init();
   res += hal_time_init();
   res += hal_console_init();
   res += hal_gpio_init();

   esp_chip_info(&chip_info);
   
   TRACE("ESP32 chip with %d CPU cores, WiFi%s%s, silicon revision %d, %dMB %s flash", 
      chip_info.cores,
      (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
      (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
      chip_info.revision,
      spi_flash_get_chip_size() / (1024 * 1024),
      (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
   
   nvs_flash_init();
   
   return 0;
}

/** Deinitialize board */
int board_deinit(void)
{
}


/** Main wrapper */
void app_main(void)
{
   extern int main(void);
   main();
}
