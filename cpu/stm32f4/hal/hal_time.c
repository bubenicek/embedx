
#include <sys/time.h>
#include "system.h"


#if defined(CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API == 1)

//
// CMSIS OS
//

int hal_time_init(void)
{
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

#else

//
// Sysless
//

typedef struct
{
   uint32_t tmo;
   uint16_t interval;
   void (*func_cb)(void);

} hal_periodic_timer_t;

static volatile uint32_t current_clock = 0;
static hal_periodic_timer_t periodic_timer;

int hal_time_init(void)
{
   SysTick_Config(SystemCoreClock / 1000);
   periodic_timer.interval = 0;
   return 0;
}

int hal_time_deinit(void)
{
   // Clear sys tick timer settings
   SysTick->CTRL = 0;
   return 0;
}

hal_time_t hal_time_ms(void)
{
   return current_clock;
}

uint32_t hal_time_us(void)
{
   return current_clock * 1000;
}

void hal_delay_ms(hal_time_t ms)
{
   ms += current_clock;
   while (current_clock < ms);
}

int hal_time_set_periodic_timer(uint16_t interval, void (*timer_cb)(void))
{
   periodic_timer.tmo = hal_time_ms() + interval;
   periodic_timer.func_cb = timer_cb;
   periodic_timer.interval = interval;

   return 0;
}

void SysTick_Handler(void)
{
   current_clock++;
   if (periodic_timer.tmo > 0 && current_clock >= periodic_timer.tmo)
   {
      periodic_timer.tmo = current_clock + periodic_timer.interval;
      if (periodic_timer.func_cb != NULL)
         periodic_timer.func_cb();
   }
}

#endif   // CFG_CMSIS_OS_API


int _gettimeofday(struct timeval *tv, struct timezone *tz)
{
   if (tv != NULL)
   {
      tv->tv_sec = hal_time_ms() / 1000;
      tv->tv_usec = 0;
   }

   return 0;
}

