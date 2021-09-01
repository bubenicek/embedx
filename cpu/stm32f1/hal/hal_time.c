
#include "system.h"
#include "dwt_delay.h"

//TRACE_TAG(hal-time);
//#if ! ENABLE_TRACE_HAL
//#undef TRACE
//#define TRACE(...)
//#endif

#if defined(CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API == 1)
//
// CMSIS OS
//

int hal_time_init(void)
{
   DWT_Init();
   return 0;
}

int hal_time_deinit(void)
{
   return 0;
}

hal_time_t hal_time_ms(void)
{
   return osKernelSysTick();
}

uint32_t hal_time_us(void)
{
   return osKernelSysTick() * 1000;
}

void hal_delay_ms(hal_time_t ms)
{
   osDelay(ms);
}

void hal_delay_us(hal_time_t us)
{
   DWT_Delay(us);
}


//
// STM32 HAL overload weak functions
//
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
   // Dont initialize systick timer
   return HAL_OK;
}

uint32_t HAL_GetTick(void)
{
   return osKernelSysTick();
}

#else

int hal_time_init(void)
{
   HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
   HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
   HAL_NVIC_SetPriority(SysTick_IRQn, CFG_HAL_SYSTICK_PRIORITY, 0);

   return 0;
}

hal_time_t hal_time_ms(void)
{
  return HAL_GetTick();
}

void hal_delay_ms(hal_time_t ms)
{
   volatile hal_time_t tmo = HAL_GetTick() + ms;
   while(HAL_GetTick() < tmo);
}
 
void SysTick_Handler(void)
{
   HAL_IncTick();
#if defined (CFG_OPENOS_OS_API) && (CFG_OPENOS_OS_API == 1)
   hal_timer_cb();
#endif   
}

#endif  // CFG_CMSIS_OS_API