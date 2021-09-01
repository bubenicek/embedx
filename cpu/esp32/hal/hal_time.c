
#include "system.h"

TRACE_TAG(hal_time);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

int hal_time_init(void)
{
   return 0;
}

hal_time_t hal_time_ms(void)
{
   if (xPortInIsrContext())
      return xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
   else
      return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void hal_delay_ms(hal_time_t ms)
{
   vTaskDelay(ms / portTICK_PERIOD_MS);
}
