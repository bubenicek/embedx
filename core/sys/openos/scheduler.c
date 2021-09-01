/**
\brief OpenOS scheduler.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include <string.h>
#include "system.h"
#include "openos/scheduler.h"

TRACE_TAG(os_scheduler);

static os_task_queue_t taskq[CFG_OS_NUM_PRIORITIES];
static int taskq_count = 0;
static int taskq_max = 0;


int os_scheduler_schedule(void)
{
   int count = 0;
	os_task_t *task;
	os_task_queue_t *tq;

  	// Process all tasks in the priors queues
  	for (tq = &taskq[0]; tq <= &taskq[CFG_OS_NUM_PRIORITIES-1]; tq++)
   {
      while (tq->tail != tq->head)
		{
         task = &tq->taskbuf[tq->tail];

	      // Execute the current task
         task->cb(task->arg);

         // Free up this task container
			tq->tail = (tq->tail + 1) & (CFG_OS_MAX_TASKS-1);
			taskq_count--;
			count++;
      }
   }
   
   return count;
}

int os_scheduler_push_task(os_task_cbt cb, os_task_prio_t prio, void *arg)
{
	uint8_t nxthead;
	os_task_t *task;
	os_task_queue_t *tq;

   ASSERT(prio < CFG_OS_NUM_PRIORITIES);
	tq = &taskq[prio];

	DISABLE_INTERRUPTS();
	
	nxthead = (tq->head + 1) & (CFG_OS_MAX_TASKS-1);
	if (nxthead == tq->tail)
	{
	   // Task priority overflow
		goto fail;
	}

	// Fill that task container with this task
	task = &tq->taskbuf[tq->head];
   task->cb = cb;
   task->prio = prio;
   task->arg = arg;

	taskq_count++;
	if (taskq_count > taskq_max)
		taskq_max = taskq_count;

	tq->head = nxthead;
	
	ENABLE_INTERRUPTS();
	
	return 0;
	
fail:
   ENABLE_INTERRUPTS();
   return -1;
}
