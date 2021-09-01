
#ifndef __ZW_TIMER_H__
#define __ZW_TIMER_H__

/**
 * A timer.
 * This structure is used for declaring a timer. The timer must be set
 * with timer_set() before it can be used.
 */
typedef struct
{
  uint32_t start;
  uint32_t interval;

}zw_timer_t;

void zw_timer_set(zw_timer_t *t, uint32_t interval);

void zw_timer_reset(zw_timer_t *t);

void zw_timer_restart(zw_timer_t *t);

int zw_timer_expired(zw_timer_t *t);

#endif /* __ZW_TIMER_H__ */
