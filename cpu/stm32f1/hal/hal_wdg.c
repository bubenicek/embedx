
#include "system.h"

#if CFG_HAL_WDG_ENABLED


int hal_wdg_init(void)
{
   return 0;
}

void hal_wdg_reset(void)
{
}

uint8_t hal_wdg_reset_occured(void)
{
   return 0;
}

#endif   // CFG_HAL_WDG_ENABLED
