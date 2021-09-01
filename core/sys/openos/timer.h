
#ifndef __OS_TIMER_H
#define __OS_TIMER_H

#ifndef CFG_OS_TIMER_PRIORITY
#define CFG_OS_TIMER_PRIORITY 		OS_TASKPRIO_MAX
#endif


struct os_timer;

typedef enum
{
	OS_TIMER_ONESHOT,
	OS_TIMER_PERIODICAL,
	OS_TIMER_IRQ_ONESHOT,
	OS_TIMER_IRQ_PERIODICAL,

} os_timer_type_t;


typedef struct os_timer
{
	TAILQ_ENTRY(os_timer) entry;
	os_timer_type_t type;
	uint32_t start;
	uint32_t interval;
	os_task_cbt task_cb;
	void *arg;

} os_timer_t;


/** Create and start timer */
int os_timer_start(os_timer_t *timer, os_timer_type_t type, uint32_t interval, os_task_cbt task_cb, void *arg);

/** Stop timer */
int os_timer_stop(os_timer_t *timer);

/** Reset running timer */
int os_timer_reset(os_timer_t *timer);


#endif // __OS_TIMER_H