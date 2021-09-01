
#include "voip_platform.h"

void voip_timer_set(voip_timer_t *t, voip_clock_time_t interval)
{
  t->interval = interval;
  t->start = voip_clock_time();
}

void voip_timer_reset(voip_timer_t *t)
{
  t->start += t->interval;
}

void voip_timer_restart(voip_timer_t *t)
{
  t->start = voip_clock_time();
}

int voip_timer_expired(voip_timer_t *t)
{
    return (voip_clock_time_t)(voip_clock_time() - t->start) >= (voip_clock_time_t)t->interval;
}
