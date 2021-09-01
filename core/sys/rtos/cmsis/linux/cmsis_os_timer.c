/**
 * \brief Timer using always new thread per timer period
 */

#include "system.h"

#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <errno.h>
#include <sched.h>

#include "cmsis_os.h"

TRACE_TAG(cmsis_timer);


/** System timer */
typedef struct systimer
{
   struct systimer *next;
   timer_t timer;
   uint8_t running;
   os_timer_type timer_type;
   uint32_t interval;
   os_ptimer timer_cb;
   void *arg;

} systimer_t;


// Locals:
static pthread_mutex_t mutex;
LIST(timers);

/** Timer callback */
static void timer_cb(union sigval arg)
{
   systimer_t *t;

   pthread_mutex_lock(&mutex);

   // Find existing timer object
   for (t = list_head(timers); t != NULL; t = list_item_next(t))
   {
      if (t == arg.sival_ptr)
         break;
   }

   pthread_mutex_unlock(&mutex);

   if (t != NULL)
   {
      if (t->timer_type == osTimerOnce)
         osTimerStop(t);

      // Invoke callback can't be in mutex
      t->timer_cb(t->arg);
   }
}


/** Initialize kernel timers */
osStatus osTimerKernelInit(void)
{
   pthread_mutexattr_t mta;

   // Initialize recursive mutex
   pthread_mutexattr_init(&mta);
   pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
   pthread_mutex_init(&mutex, &mta);

   list_init(timers);

   return osOK;
}


/**
* @brief  Create a timer.
* @param  timer_def     timer object referenced with \ref osTimer.
* @param  type          osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
* @param  argument      argument to the timer call back function.
* @retval  timer ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
*/
osTimerId osTimerCreate(const osTimerDef_t *timer_def, os_timer_type type, void *argument)
{
   systimer_t *t;
   struct sigevent evp;

   if ((t = osMemAlloc(sizeof(systimer_t))) == NULL)
   {
      TRACE_ERROR("Alloc timer");
      throw_exception(fail_alloc);
   }

   memset(t, 0, sizeof(systimer_t));
   t->timer_type =  type;
   t->arg = argument;
   t->timer_cb = timer_def->ptimer;

   memset((void *)&evp, 0, sizeof(evp));
   evp.sigev_notify = SIGEV_THREAD;
   evp.sigev_notify_function = &timer_cb;
   evp.sigev_value.sival_ptr = t;

   pthread_mutex_lock(&mutex);

   if (timer_create(CLOCK_REALTIME, &evp, &t->timer) != 0)
      throw_exception(fail_create_timer);

   list_add(timers, t);

   pthread_mutex_unlock(&mutex);

   return t;

fail_create_timer:
   osMemFree(t);
   pthread_mutex_unlock(&mutex);
fail_alloc:
   return NULL;
}

/**
  * @brief  Delete a timer.
  * @note   MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
  */
osStatus osTimerDelete(osTimerId timer_id)
{
   systimer_t *t = timer_id;

   pthread_mutex_lock(&mutex);

   if (t->running)
      osTimerStop(timer_id);

   VERIFY(timer_delete(t->timer) == 0);

   list_remove(timers, t);
   osMemFree(t);

   pthread_mutex_unlock(&mutex);

   return osOK;
}

/**
* @brief  Start or restart a timer.
* @param  timer_id      timer ID obtained by \ref osTimerCreate.
* @param  millisec      time delay value of the timer.
* @retval  status code that indicates the execution status of the function
* @note   MUST REMAIN UNCHANGED: \b osTimerStart shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerStart(osTimerId timer_id, uint32_t millisec)
{
   struct itimerspec ts;
   systimer_t *t = timer_id;

   pthread_mutex_lock(&mutex);

   if (t->running)
      osTimerStop(timer_id);

   ts.it_value.tv_sec = millisec / 1000;
   ts.it_value.tv_nsec = (millisec % 1000) * 1000000;
   ts.it_interval.tv_sec = ts.it_value.tv_sec;
   ts.it_interval.tv_nsec = ts.it_value.tv_nsec;

   if (timer_settime(t->timer, 0, &ts, NULL) != 0)
      throw_exception(fail);

   t->running = 1;
   t->interval = millisec;

   pthread_mutex_unlock(&mutex);

   return osOK;

fail:
   pthread_mutex_unlock(&mutex);
   return osErrorOS;
}

/**
* @brief  Stop a timer.
* @param  timer_id      timer ID obtained by \ref osTimerCreate
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osTimerStop shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerStop (osTimerId timer_id)
{
   struct itimerspec ts;
   systimer_t *t = timer_id;

   pthread_mutex_lock(&mutex);

   if (!t->running)
      throw_exception(fail);

   memset(&ts, 0, sizeof(ts));
   t->running = 0;

   if (timer_settime(t->timer, 0, &ts, NULL) != 0)
      throw_exception(fail);

   pthread_mutex_unlock(&mutex);

   return osOK;

fail:
   pthread_mutex_unlock(&mutex);
   return osErrorOS;
}
