
#include "system.h"
#include "openos/scheduler.h"
#include "openos/timer.h"

TRACE_TAG(os_timer);

// Locals:
static TAILQ_HEAD(os_timer_queue, os_timer) timers = TAILQ_HEAD_INITIALIZER(timers);


/** Create and start timer */
int os_timer_start(os_timer_t *timer, os_timer_type_t type, uint32_t interval, os_task_cbt task_cb, void *arg)
{
   struct os_timer *cur;

   DISABLE_INTERRUPTS();

	// Initialize timer
	timer->type = type;
	timer->start = hal_time_ms();
	timer->interval = interval;
	timer->task_cb = task_cb;
   timer->arg = arg;

   // Find exists timer
	TAILQ_FOREACH(cur, &timers, entry)
	{
      if (cur == timer)
         break;
   }

	// If not exists add to queue
   if (cur == NULL)
      TAILQ_INSERT_TAIL(&timers, timer, entry);

   ENABLE_INTERRUPTS();

   return 0;
}

/** Stop timer */
int os_timer_stop(os_timer_t *timer)
{
	int res = -1;
   struct os_timer *cur;

   DISABLE_INTERRUPTS();

   // Remove timer from timers queue
   TAILQ_FOREACH(cur, &timers, entry)
   {
      if (cur == timer)
		{
         TAILQ_REMOVE(&timers, cur, entry);
			res = 0;
			break;
      }
   }

   ENABLE_INTERRUPTS();

	return res;
}

/** Reset running timer */
int os_timer_reset(os_timer_t *timer)
{
	int res = -1;
   struct os_timer *cur;

   DISABLE_INTERRUPTS();

   TAILQ_FOREACH(cur, &timers, entry)
   {
      if (cur == timer)
		{
         //cur->start += cur->interval;
         cur->start = hal_time_ms();
			res = 0;
			break;
      }
	}

   ENABLE_INTERRUPTS();

   return res;
}

void hal_timer_cb(void)
{
	struct os_timer *cur, *tmp;

   DISABLE_INTERRUPTS();

	// Remove timer from timers list
	TAILQ_FOREACH(cur, &timers, entry)
	{
		if (hal_time_ms() - cur->start >= cur->interval)
		{
		   if (cur->type == OS_TIMER_ONESHOT || cur->type == OS_TIMER_PERIODICAL)
         {
            // Schedule timer task callback
            os_scheduler_push_task(cur->task_cb, CFG_OS_TIMER_PRIORITY, cur->arg);
         }
         else
         {
            // Immediate invoke in IRQ handler
            cur->task_cb(cur->arg);
         }

			if (cur->type == OS_TIMER_ONESHOT)
			{
            // Oneshot timer - Remove timer from the queue
            tmp = TAILQ_NEXT(cur, entry);
            TAILQ_REMOVE(&timers, cur, entry);
            cur = tmp;
            if (cur == NULL)
               break;
			}
			else
			{
				// Periodically - reset timer
				cur->start += cur->interval;
			}
		}
	}

   ENABLE_INTERRUPTS();
}
