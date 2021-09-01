
#include <cmsis_os.h>
#include "zw_timer.h"

/**
 * Set a timer.
 *
 * This function is used to set a timer for a time sometime in the
 * future. The function timer_expired() will evaluate to true after
 * the timer has expired.
 *
 * \param t A pointer to the timer
 * \param interval The interval before the timer expires.
 *
 */
void zw_timer_set(zw_timer_t *t, uint32_t interval)
{
  t->interval = interval;
  t->start = osKernelSysTick();
}

/**
 * Reset the timer with the same interval.
 *
 * This function resets the timer with the same interval that was
 * given to the timer_set() function. The start point of the interval
 * is the exact time that the timer last expired. Therefore, this
 * function will cause the timer to be stable over time, unlike the
 * timer_rester() function.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_restart()
 */
void zw_timer_reset(zw_timer_t *t)
{
  t->start += t->interval;
}

/**
 * Restart the timer from the current point in time
 *
 * This function restarts a timer with the same interval that was
 * given to the timer_set() function. The timer will start at the
 * current time.
 *
 * \note A periodic timer will drift if this function is used to reset
 * it. For preioric timers, use the timer_reset() function instead.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_reset()
 */
void zw_timer_restart(zw_timer_t *t)
{
  t->start = osKernelSysTick();
}

/**
 * Check if a timer has expired.
 *
 * This function tests if a timer has expired and returns true or
 * false depending on its status.
 *
 * \param t A pointer to the timer
 *
 * \return Non-zero if the timer has expired, zero otherwise.
 *
 */
int zw_timer_expired(zw_timer_t *t)
{
  return (osKernelSysTick() - t->start >= t->interval);
}

