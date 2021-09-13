
#include "system.h"

#if CFG_HAL_WDG_ENABLED

TRACE_TAG(wdg);

static WWDG_HandleTypeDef hwwdg;

int hal_wdg_init(void)
{
    __HAL_RCC_WWDG_CLK_ENABLE();

    hwwdg.Instance = WWDG;
    hwwdg.Init.Prescaler = WWDG_PRESCALER_1;
    hwwdg.Init.Window = 64;
    hwwdg.Init.Counter = 64;
    hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
    if (HAL_WWDG_Init(&hwwdg) != HAL_OK)
    {
        TRACE_ERROR("Watchdog init failed");
        return -1;
    }

   return 0;
}

void hal_wdg_reset(void)
{
    if (HAL_WWDG_Refresh(&hwwdg) != HAL_OK)
    {
        TRACE_ERROR("Reset watchod failed");
    }
}

uint8_t hal_wdg_reset_occured(void)
{
    // TODO:
    return 0;
}

#endif   // CFG_HAL_WDG_ENABLED
