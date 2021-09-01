
#include "system.h"

#define TRACE_TAG		"hal-time"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

static volatile uint32_t ticks = 0;

int hal_time_init(void)
{
   // Timer/Counter 0 initialization
   // Clock source: System Clock
   // Clock value: 18,000 kHz
   // Mode: CTC top=OCR0A
   // OC0A output: Disconnected
   // OC0B output: Disconnected
   TCCR0A=0x02;
   TCCR0B=0x05;
   TCNT0=0x00;
   OCR0A=15;
   OCR0B=0x00;

   // Timer/Counter 0 Interrupt(s) initialization
   TIMSK0=0x02;

   return 0;
}

hal_time_t hal_time_ms(void)
{
   return ticks;
}

void hal_delay_ms(hal_time_t ms)
{
   volatile uint32_t endtm = ticks + ms;
   while(ticks < endtm);
}

ISR(TIMER0_COMPA_vect)
{
    ticks++;
    hal_timer_cb();
}

