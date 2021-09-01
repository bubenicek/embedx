/**
 * \file hal_timer.h       \brief HW timer interface
 */

#ifndef __HAL_TIMER_H
#define __HAL_TIMER_H


/** Timers */
typedef enum
{
   HAL_TIMER0,
   HAL_TIMER1,
   HAL_TIMER2,
   HAL_TIMER3,
   HAL_TIMER_MAX

} hal_timer_t;


/** Timer types */
typedef enum  
{
   HAL_TIMER_ONCE,       ///< one-shot timer
   HAL_TIMER_PERIODIC    ///< repeating timer

} hal_timer_type_t;


/** HAL timer callback */
typedef void (*hal_timer_cb_t)(hal_timer_t tmr, void *arg);


/** Initialize timer */
int hal_timer_init(hal_timer_t tmr, hal_timer_type_t type, hal_timer_cb_t cb, void *arg);

/** Deinitialize timer */
int hal_timer_deinit(hal_timer_t tmr);

/** Return true if timer is running else false */
bool hal_timer_running(hal_timer_t tmr);

/** Start timer */
int hal_timer_start(hal_timer_t tmr, uint32_t time_usec);

/** Stop timer */
int hal_timer_stop(hal_timer_t tmr);

/** Restart timer */
inline static int hal_timer_restart(hal_timer_t tmr, uint32_t time_usec)
{
   int res = hal_timer_stop(tmr);
   res += hal_timer_start(tmr, time_usec);
   return res;
}


#endif  // __HAL_TIMER_H