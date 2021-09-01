
#ifndef __voip_timer_h
#define __voip_timer_h

typedef uint32_t voip_clock_time_t;

typedef struct
{
   voip_clock_time_t start;
   voip_clock_time_t interval;

} voip_timer_t;

void voip_timer_set(voip_timer_t *t, voip_clock_time_t interval);
void voip_timer_reset(voip_timer_t *t);
void voip_timer_restart(voip_timer_t *t);
int voip_timer_expired(voip_timer_t *t);

#endif   // voip_timer.h

