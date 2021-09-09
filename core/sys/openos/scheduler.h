
#ifndef __OS_SCHEDULER_H
#define __OS_SCHEDULER_H


#ifndef CFG_OS_MAX_TASKS
#define CFG_OS_MAX_TASKS				64
#endif

#ifndef CFG_OS_NUM_PRIORITIES
#define CFG_OS_NUM_PRIORITIES			2
#endif


/** Tasks priorities */
typedef enum
{
   OS_TASKPRIO_MAX  = 0,
   OS_TASKPRIO_NORMAL = 1,
   OS_TASKPRIO_NONE = CFG_OS_NUM_PRIORITIES - 1,

} os_task_prio_t;


/** Task callback function */
typedef void (*os_task_cbt)(void *arg);


/** Task */
typedef struct
{
   os_task_cbt cb;
   os_task_prio_t prio;
   void *arg;

} os_task_t;


typedef struct
{
	os_task_t taskbuf[CFG_OS_MAX_TASKS];
	volatile uint8_t head;
	volatile uint8_t tail;

} os_task_queue_t;


/**
 * Schedule tasks in the queue
 * @return number of scheduled tasks
*/
int os_scheduler_schedule(void);

/**
 * Add task to schedule queue
 * @return 0 if success else -1 if schedule queue is full
 */
int os_scheduler_push_task(os_task_cbt task_cb, os_task_prio_t prio, void *arg);

#endif		// __OS_SCHEDULER_H
