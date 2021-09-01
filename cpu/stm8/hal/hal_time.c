
#include "system.h"

#define TRACE_TAG		"hal-time"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#define TIM4_PERIOD       124

// Prototypes:
extern void hal_timer_cb(void);

// Locals:
static volatile uint32_t ticks = 0;

int hal_time_init(void)
{
   /* TIM4 configuration:
   - TIM4CLK is set to 16 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
   clock used is 16 MHz / 128 = 125 000 Hz
   - With 125 000 Hz we can generate time base:
      max time base is 2.048 ms if TIM4_PERIOD = 255 --> (255 + 1) / 125000 = 2.048 ms
      min time base is 0.016 ms if TIM4_PERIOD = 1   --> (  1 + 1) / 125000 = 0.016 ms
   - In this example we need to generate a time base equal to 1 ms
   so TIM4_PERIOD = (0.001 * 125000 - 1) = 124 */

   /* Time base configuration */
   TIM4_TimeBaseInit(TIM4_PRESCALER_128, TIM4_PERIOD);

   /* Clear TIM4 update flag */
   TIM4_ClearFlag(TIM4_FLAG_UPDATE);

   /* Enable update interrupt */
   TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

   /* Enable TIM4 */
   TIM4_Cmd(ENABLE);

   return 0;
}

uint32_t hal_time_ms(void)
{
   return ticks;
}

void hal_delay_ms(uint32_t ms)
{
   volatile uint32_t endtm = ticks + ms;
   while(ticks < endtm);   
}

INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)
{
   TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
   ticks++;
   hal_timer_cb();
}
