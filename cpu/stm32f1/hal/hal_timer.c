#include "system.h"

#ifdef CFG_HAL_TIMER_DEF

TRACE_TAG(hal_timer);

#ifndef CFG_HAL_TIMER_PRIORITY
#define CFG_HAL_TIMER_PRIORITY         0
#endif

// Locals:
static hal_timer_def_t timers[] = CFG_HAL_TIMER_DEF;
#define NUM_TIMERS   (sizeof(timers) / sizeof(hal_timer_def_t))


/** Initialize timer */
int hal_timer_init(hal_timer_t tmr, hal_timer_type_t type, hal_timer_cb_t cb, void *arg)
{
   ASSERT(tmr < NUM_TIMERS);

   timers[tmr].id = tmr;
   timers[tmr].type = type;
   timers[tmr].cb = cb;
   timers[tmr].arg = arg;

   // enable Clocks 
   if (timers[tmr].instance == TIM2)
   {
      __HAL_RCC_TIM2_CLK_ENABLE();
      timers[tmr].irqn = TIM2_IRQn;
   }
   else
   {
      TRACE_ERROR("Not supported HW timer instance %p", timers[tmr].instance);
      return -1;
   }   

   // Configure IRQ
   HAL_NVIC_SetPriority(timers[tmr].irqn, CFG_HAL_TIMER_PRIORITY, 0);
   HAL_NVIC_EnableIRQ(timers[tmr].irqn);

   TRACE("HW timer[%d] init", tmr);

   return 0;
}

/** Deinitialize timer */
int hal_timer_deinit(hal_timer_t tmr)
{
   ASSERT(tmr < NUM_TIMERS);

   if (timers[tmr].running)
   {
      hal_timer_stop(tmr);

      HAL_NVIC_DisableIRQ(timers[tmr].irqn);
      HAL_TIM_Base_DeInit(&timers[tmr].base);
   }

   return 0;
}

/** Return true if timer is running else false */
bool hal_timer_running(hal_timer_t tmr)
{
   ASSERT(tmr < NUM_TIMERS);
   return timers[tmr].running;
}

/** Start timer */
int hal_timer_start(hal_timer_t tmr, uint32_t time_usec)
{
   uint32_t period;
   uint32_t prescaler = 0;
   uint32_t freq_hz = 1 / ((time_usec * 2) / 1000000.0);

   ASSERT(tmr < NUM_TIMERS);

   if (timers[tmr].running)
      hal_timer_stop(tmr);

   // Calculate timer period
   period = (HAL_RCC_GetPCLK1Freq() / freq_hz);
   while (period > 0xFFFF)
   {
      prescaler += 128;
      period = (HAL_RCC_GetPCLK1Freq() / prescaler / freq_hz);
   }

   timers[tmr].base.Instance = timers[tmr].instance;
   timers[tmr].base.Init.CounterMode = TIM_COUNTERMODE_UP;
   timers[tmr].base.Init.Period = period;
   timers[tmr].base.Init.Prescaler = prescaler;
   timers[tmr].base.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
   timers[tmr].base.Init.RepetitionCounter = 0;
  
   HAL_TIM_Base_Init(&timers[tmr].base); 
   HAL_TIM_Base_Start_IT(&timers[tmr].base); 

   TRACE("HW timer[%d] start - freq_hz: %d  period: %d  prescaler: %d  time_usec: %d", tmr, freq_hz, period, prescaler, time_usec);

   return 0;
}

/** Stop timer */
int hal_timer_stop(hal_timer_t tmr)
{
   ASSERT(tmr < NUM_TIMERS);

   DISABLE_INTERRUPTS();

   if (timers[tmr].running)
   {
      HAL_TIM_Base_Stop_IT(&timers[tmr].base);
      timers[tmr].running = false;
   }

   ENABLE_INTERRUPTS();

   return 0;
}

void TIM2_IRQHandler(void)
{
   for (int i = 0; i < NUM_TIMERS; i++)
   {
      if (timers[i].base.Instance == TIM2)
      {
         HAL_TIM_IRQHandler(&timers[i].base);
         break;
      } 
   }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
   for (int i = 0; i < NUM_TIMERS; i++)
   {
      if (&timers[i].base == htim)
      {
         if (timers[i].type == HAL_TIMER_ONCE)
            hal_timer_stop(timers[i].id);

         if (timers[i].cb != NULL)
            timers[i].cb(timers[i].id, timers[i].arg);

         break;
      }
   }
}

#endif   // CFG_HAL_TIMER_DEF