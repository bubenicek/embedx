
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <process.h>
#include <intrin.h>
#include "cmsis_os.h"


typedef unsigned (__stdcall *winapi_thread_t) (void *);

/*********************** Kernel Control Functions *****************************/
/**
* @brief  Start the RTOS Kernel with executing the specified thread.
* @param  thread_def    thread definition referenced with \ref osThread.
* @param  argument      pointer that is passed to the thread function as start argument.
* @retval status code that indicates the execution status of the function
* @note   MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
*/
osStatus osKernelStart (osThreadDef_t *thread_def, void *argument)
{
   if (thread_def != NULL)
   {
      osThreadCreate(thread_def, argument);
   }

   // Suspend main thread
   SuspendThread(GetCurrentThread());

   return osOK;
}

/**
* @brief  Get the value of the Kernel SysTick timer
* @param  None
* @retval None
* @note   MUST REMAIN UNCHANGED: \b osKernelSysTick shall be consistent in every CMSIS-RTOS.
*/
uint32_t osKernelSysTick(void)
{
   static uint32_t start_time = 0;

   if (!start_time)
   {
      start_time = GetTickCount();
   }

   return GetTickCount() - start_time;
}


/**
* @brief  Check if the RTOS kernel is already started
* @param  None
* @retval (0) RTOS is not started
*         (1) RTOS is started
*         (-1) if this feature is disabled in FreeRTOSConfig.h
* @note  MUST REMAIN UNCHANGED: \b osKernelRunning shall be consistent in every CMSIS-RTOS.
*/
int32_t osKernelRunning(void)
{
   return 1;
}


/*********************** Thread Management *****************************/
/**
* @brief  Create a thread and add it to Active Threads and set it to state READY.
* @param  thread_def    thread definition referenced with \ref osThread.
* @param  argument      pointer that is passed to the thread function as start argument.
* @retval thread ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
*/
osThreadId osThreadCreate (osThreadDef_t *thread_def, void *argument)
{
	DWORD threadId;

   return CreateThread(
               NULL,
               0,
               (LPTHREAD_START_ROUTINE)thread_def->pthread,
               argument,
               0,  // CREATE_SUSPENDED
               &threadId);
}


/**
* @brief  Return the thread ID of the current running thread.
* @retval thread ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
*/
osThreadId osThreadGetId (void)
{
   return GetCurrentThread();
}


/**
* @brief  Terminate execution of a thread and remove it from Active Threads.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadTerminate (osThreadId thread_id)
{
   return TerminateThread(thread_id, 0) == TRUE ? osOK : osErrorOS;
}

/**
* @brief  Pass control to next thread that is in state \b READY.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadYield (void)
{
   // Not impl.
   return osOK;
}

/**
* @brief   Change priority of an active thread.
* @param   thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @param   priority      new priority value for the thread function.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority)
{
   return SetThreadPriority(thread_id, priority) == TRUE ? osOK : osErrorOS;
}

/**
* @brief   Get current priority of an active thread.
* @param   thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  current priority value of the thread function.
* @note   MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
*/
osPriority osThreadGetPriority (osThreadId thread_id)
{
   return GetThreadPriority(thread_id);
}

/*********************** Generic Wait Functions *******************************/
/**
* @brief   Wait for Timeout (Time Delay)
* @param   millisec      time delay value
* @retval  status code that indicates the execution status of the function.
*/
osStatus osDelay (uint32_t millisec)
{
   Sleep(millisec);
   return osOK;
}

#if (defined (osFeature_Wait)  &&  (osFeature_Wait != 0)) /* Generic Wait available */
/**
* @brief  Wait for Signal, Message, Mail, or Timeout
* @param   millisec  timeout value or 0 in case of no time-out
* @retval  event that contains signal, message, or mail information or error code.
* @note   MUST REMAIN UNCHANGED: \b osWait shall be consistent in every CMSIS-RTOS.
*/
osEvent osWait (uint32_t millisec);

#endif  /* Generic Wait available */


/***********************  Timer Management Functions ***************************/

typedef struct
{
   HANDLE htimer;
   bool running;

   os_timer_type timer_type;
   uint32_t interval;
   os_ptimer timer_cb;
   void *user_param;

} waitable_timer_t;


//callback
static void __stdcall timer_callback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
   waitable_timer_t *t = lpParameter;

   if (t->running)
   {
      t->timer_cb(t->user_param);
   }
}

/**
* @brief  Create a timer.
* @param  timer_def     timer object referenced with \ref osTimer.
* @param  type          osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
* @param  argument      argument to the timer call back function.
* @retval  timer ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
*/
osTimerId osTimerCreate (osTimerDef_t *timer_def, os_timer_type type, void *argument)
{
   waitable_timer_t *t;

   t = malloc(sizeof(waitable_timer_t));
   if (t == NULL)
   {
      return NULL;
   }
   memset(t, 0, sizeof(waitable_timer_t));

   t->timer_type =  type;
   t->user_param = argument;
   t->timer_cb = timer_def->ptimer;

   return t;
}

/**
  * @brief  Delete a timer.
  * @note   MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
  */
osTimerId osTimerDelete(osTimerId timer_id)
{
   waitable_timer_t *t = timer_id;

   if (t->running)
   {
      osTimerStop(timer_id);
   }

   free(t);

   return osOK;
}


/**
* @brief  Start or restart a timer.
* @param  timer_id      timer ID obtained by \ref osTimerCreate.
* @param  millisec      time delay value of the timer.
* @retval  status code that indicates the execution status of the function
* @note   MUST REMAIN UNCHANGED: \b osTimerStart shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerStart (osTimerId timer_id, uint32_t millisec)
{
   waitable_timer_t *t = timer_id;

   if (t->running)
   {
      osTimerStop(timer_id);
   }

   if (!CreateTimerQueueTimer(&t->htimer, NULL, timer_callback, t, millisec, millisec, WT_EXECUTEDEFAULT))
   {
      return osErrorOS;
   }

   t->running = true;

   return osOK;
}

/**
* @brief  Stop a timer.
* @param  timer_id      timer ID obtained by \ref osTimerCreate
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osTimerStop shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerStop (osTimerId timer_id)
{
   waitable_timer_t *t = timer_id;

   if (!t->running)
   {
      return osErrorOS;
   }

   t->running = false;
   if (!DeleteTimerQueueTimer(NULL, t->htimer, NULL))
   {
      if (GetLastError() != ERROR_IO_PENDING)
      {
         return osErrorOS;
      }
   }

   return osOK;
}

/***************************  Signal Management ********************************/
/**
* @brief  Set the specified Signal Flags of an active thread.
* @param  thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @param  signals       specifies the signal flags of the thread that should be set.
* @retval  previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
* @note   MUST REMAIN UNCHANGED: \b osSignalSet shall be consistent in every CMSIS-RTOS.
*/
int32_t osSignalSet (osThreadId thread_id, int32_t signal);

/**
* @brief  Clear the specified Signal Flags of an active thread.
* @param  thread_id  thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @param  signals    specifies the signal flags of the thread that shall be cleared.
* @retval  previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
* @note   MUST REMAIN UNCHANGED: \b osSignalClear shall be consistent in every CMSIS-RTOS.
*/
int32_t osSignalClear (osThreadId thread_id, int32_t signal);

/**
* @brief  Get Signal Flags status of an active thread.
* @param  thread_id  thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
* @note   MUST REMAIN UNCHANGED: \b osSignalGet shall be consistent in every CMSIS-RTOS.
*/
int32_t osSignalGet (osThreadId thread_id);

/**
* @brief  Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
* @param  signals   wait until all specified signal flags set or 0 for any single signal flag.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval  event flag information or error code.
* @note   MUST REMAIN UNCHANGED: \b osSignalWait shall be consistent in every CMSIS-RTOS.
*/
osEvent osSignalWait (int32_t signals, uint32_t millisec);

/****************************  Mutex Management ********************************/

/**
* @brief  Create and Initialize a Mutex object
* @param  mutex_def     mutex definition referenced with \ref osMutex.
* @retval  mutex ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osMutexCreate shall be consistent in every CMSIS-RTOS.
*/
osMutexId osMutexCreate (osMutexDef_t *mutex_def)
{
   return CreateMutex(NULL, FALSE, NULL);
}


/**
* @brief Wait until a Mutex becomes available
* @param mutex_id      mutex ID obtained by \ref osMutexCreate.
* @param millisec      timeout value or 0 in case of no time-out.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexWait shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec)
{
   osStatus ret;
   DWORD dwWaitResult;

   dwWaitResult = WaitForSingleObject(mutex_id, millisec);
   switch (dwWaitResult)
   {
      case WAIT_OBJECT_0:
         ret = osOK;
         break;

      case WAIT_TIMEOUT:
         ret = osErrorTimeoutResource;
         break;

      default:
         ret = osErrorOS;
   }

   return ret;
}

/**
* @brief Release a Mutex that was obtained by \ref osMutexWait
* @param mutex_id      mutex ID obtained by \ref osMutexCreate.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexRelease (osMutexId mutex_id)
{
   return ReleaseMutex(mutex_id) ? osOK : osErrorOS;
}

/**
* @brief Delete a Mutex
* @param mutex_id  mutex ID obtained by \ref osMutexCreate.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexDelete (osMutexId mutex_id)
{
   return CloseHandle(mutex_id) ? osOK : osErrorOS;
}

/********************  Semaphore Management Functions **************************/

#if (defined (osFeature_Semaphore)  &&  (osFeature_Semaphore != 0))

/**
* @brief Create and Initialize a Semaphore object used for managing resources
* @param semaphore_def semaphore definition referenced with \ref osSemaphore.
* @param count         number of available resources.
* @retval  semaphore ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreCreate shall be consistent in every CMSIS-RTOS.
*/
osSemaphoreId osSemaphoreCreate (osSemaphoreDef_t *semaphore_def, int32_t count)
{
   HANDLE semaphore_id;

   semaphore_id = CreateSemaphore(
                     NULL, // default security attributes
                     0,    // initial count
                     count, // maximum count
                     NULL); // named semaphore

   return semaphore_id;
}

/**
* @brief Wait until a Semaphore token becomes available
* @param  semaphore_id  semaphore object referenced with \ref osSemaphore.
* @param  millisec      timeout value or 0 in case of no time-out.
* @retval  number of available tokens, or -1 in case of incorrect parameters.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreWait shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec)
{
   osStatus ret;

   DWORD dwWaitResult;

   dwWaitResult = WaitForSingleObject(semaphore_id, millisec);
   switch (dwWaitResult)
   {
      // The semaphore object was signaled.
      case WAIT_OBJECT_0:
         ret = osOK;
         break;

      // Semaphore was nonsignaled, so a time-out occurred.
      case WAIT_TIMEOUT:
         ret = osErrorTimeoutResource;
         break;

      default:
         ret = osErrorOS;
   }

   return ret;
}

/**
* @brief Release a Semaphore token
* @param  semaphore_id  semaphore object referenced with \ref osSemaphore.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id)
{
   // Increment the count of the semaphore.
   return ReleaseSemaphore(semaphore_id, 1, NULL) == TRUE ? osOK : osErrorOS;
}

/**
* @brief Delete a Semaphore
* @param  semaphore_id  semaphore object referenced with \ref osSemaphore.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{
   return CloseHandle(semaphore_id) ? osOK : osErrorOS;
}

#endif    /* Use Semaphores */

/*******************   Memory Pool Management Functions  ***********************/

#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))

//TODO
//This is a primitive and inefficient wrapper around the existing FreeRTOS memory management.
//A better implementation will have to modify heap_x.c!


typedef struct os_pool_cb
{
   void *pool;
   uint8_t *markers;
   uint32_t pool_sz;
   uint32_t item_sz;
   uint32_t currentIndex;
} os_pool_cb_t;


/**
* @brief Create and Initialize a memory pool
* @param  pool_def      memory pool definition referenced with \ref osPool.
* @retval  memory pool ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osPoolCreate shall be consistent in every CMSIS-RTOS.
*/
osPoolId osPoolCreate (osPoolDef_t *pool_def)
{
   osPoolId thePool;
   int itemSize = 4 * ((pool_def->item_sz + 3) / 4);
   uint32_t i;

   /* First have to allocate memory for the pool control block. */
   thePool = pvPortMalloc(sizeof(os_pool_cb_t));
   if (thePool)
   {
      thePool->pool_sz = pool_def->pool_sz;
      thePool->item_sz = itemSize;
      thePool->currentIndex = 0;

      /* Memory for markers */
      thePool->markers = pvPortMalloc(pool_def->pool_sz);
      if (thePool->markers)
      {
         /* Now allocate the pool itself. */
         thePool->pool = pvPortMalloc(pool_def->pool_sz * itemSize);

         if (thePool->pool)
         {
            for (i = 0; i < pool_def->pool_sz; i++)
            {
               thePool->markers[i] = 0;
            }
         }
         else
         {
            vPortFree(thePool->markers);
            vPortFree(thePool);
            thePool = NULL;
         }
      }
      else
      {
         vPortFree(thePool);
         thePool = NULL;
      }
   }

   return thePool;
}

/**
* @brief Allocate a memory block from a memory pool
* @param pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
* @retval  address of the allocated memory block or NULL in case of no memory available.
* @note   MUST REMAIN UNCHANGED: \b osPoolAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osPoolAlloc (osPoolId pool_id)
{
   int dummy = 0;
   void *p = NULL;
   uint32_t i;
   uint32_t index;

   if (inHandlerMode())
   {
      dummy = portSET_INTERRUPT_MASK_FROM_ISR();
   }
   else
   {
      vPortEnterCritical();
   }

   for (i = 0; i < pool_id->pool_sz; i++)
   {
      index = pool_id->currentIndex + i;
      if (index >= pool_id->pool_sz)
      {
         index = 0;
      }

      if (pool_id->markers[index] == 0)
      {
         pool_id->markers[index] = 1;
         p = (void *)((uint32_t)(pool_id->pool) + (index * pool_id->item_sz));
         pool_id->currentIndex = index;
         break;
      }
   }

   if (inHandlerMode())
   {
      portCLEAR_INTERRUPT_MASK_FROM_ISR(dummy);
   }
   else
   {
      vPortExitCritical();
   }

   return p;
}

/**
* @brief Allocate a memory block from a memory pool and set memory block to zero
* @param  pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
* @retval  address of the allocated memory block or NULL in case of no memory available.
* @note   MUST REMAIN UNCHANGED: \b osPoolCAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osPoolCAlloc (osPoolId pool_id)
{
   void *p = osPoolAlloc(pool_id);

   if (p != NULL)
   {
      memset(p, 0, sizeof(pool_id->pool_sz));
   }

   return p;
}

/**
* @brief Return an allocated memory block back to a specific memory pool
* @param  pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
* @param  block         address of the allocated memory block that is returned to the memory pool.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osPoolFree shall be consistent in every CMSIS-RTOS.
*/
osStatus osPoolFree (osPoolId pool_id, void *block)
{
   uint32_t index;

   if (pool_id == NULL)
   {
      return osErrorParameter;
   }

   if (block == NULL)
   {
      return osErrorParameter;
   }

   if (block < pool_id->pool)
   {
      return osErrorParameter;
   }

   index = (uint32_t)block - (uint32_t)(pool_id->pool);
   if (index % pool_id->item_sz)
   {
      return osErrorParameter;
   }
   index = index / pool_id->item_sz;
   if (index >= pool_id->pool_sz)
   {
      return osErrorParameter;
   }

   pool_id->markers[index] = 0;

   return osOK;
}


#endif   /* Use Memory Pool Management */

/*******************   Message Queue Management Functions  *********************/

#if (defined (osFeature_MessageQ)  &&  (osFeature_MessageQ != 0)) /* Use Message Queues */

typedef struct
{
   osMessageQDef_t def;
   volatile uint16_t head;
   volatile uint16_t tail;
   uint8_t *buffer;

} message_queue_t;


/**
* @brief Create and Initialize a Message Queue
* @param queue_def     queue definition referenced with \ref osMessageQ.
* @param  thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
* @retval  message queue ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osMessageCreate shall be consistent in every CMSIS-RTOS.
*/
osMessageQId osMessageCreate (osMessageQDef_t *queue_def, osThreadId thread_id)
{
   message_queue_t *mq;

   mq = malloc(sizeof(message_queue_t) + queue_def->queue_sz * queue_def->item_sz);
   if (mq == NULL)
   {
      return NULL;
   }
   memset(mq, 0, sizeof(message_queue_t) + queue_def->queue_sz * queue_def->item_sz);

   //memcpy(&mq->def, queue_def, sizeof(osMessageQDef_t));
   mq->tail = mq->head = 0;

   return mq;
}

/**
* @brief Put a Message to a Queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  info      message information.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMessagePut shall be consistent in every CMSIS-RTOS.
*/
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
   return osOK;
}

/**
* @brief Get a Message or Wait for a Message from a Queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval event information that includes status code.
* @note   MUST REMAIN UNCHANGED: \b osMessageGet shall be consistent in every CMSIS-RTOS.
*/
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec)
{
/*
   portBASE_TYPE taskWoken;
   portTickType ticks;
   osEvent event;

   event.def.message_id = queue_id;

   if (queue_id == NULL)
   {
      event.status = osErrorParameter;
      return event;
   }

   taskWoken = pdFALSE;

   ticks = 0;
   if (millisec == osWaitForever)
   {
      ticks = portMAX_DELAY;
   }
   else if (millisec != 0)
   {
      ticks = millisec / portTICK_RATE_MS;
      if (ticks == 0)
      {
         ticks = 1;
      }
   }

   if (inHandlerMode())
   {
      if (xQueueReceiveFromISR(queue_id, &event.value.v, &taskWoken) == pdTRUE)
      {
         // We have mail
         event.status = osEventMessage;
      }
      else
      {
         event.status = osOK;
      }
      portEND_SWITCHING_ISR(taskWoken);
   }
   else
   {
      if (xQueueReceive(queue_id, &event.value.v, ticks) == pdTRUE)
      {
         // We have mail
         event.status = osEventMessage;
      }
      else
      {
         event.status = (ticks == 0) ? osOK : osEventTimeout;
      }
   }
*/
   osEvent event;
   return event;
}

/**
* @brief  Receive an item from a queue without removing the item from the queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval event information that includes status code.
*/
osEvent osMessagePeek (osMessageQId queue_id, uint32_t millisec)
{
   osEvent event;
   return event;
}

#endif     /* Use Message Queues */

/********************   Mail Queue Management Functions  ***********************/

#if (defined (osFeature_MailQ)  &&  (osFeature_MailQ != 0))  /* Use Mail Queues */


typedef struct os_mailQ_cb
{
   osMailQDef_t *queue_def;
   xQueueHandle handle;
   osPoolId pool;
} os_mailQ_cb_t;

/**
* @brief Create and Initialize mail queue
* @param  queue_def     reference to the mail queue definition obtain with \ref osMailQ
* @param   thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
* @retval mail queue ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osMailCreate shall be consistent in every CMSIS-RTOS.
*/
osMailQId osMailCreate (osMailQDef_t *queue_def, osThreadId thread_id)
{
   (void) thread_id;

   osPoolDef_t pool_def = {queue_def->queue_sz, queue_def->item_sz};


   /* Create a mail queue control block */
   *(queue_def->cb) = pvPortMalloc(sizeof(struct os_mailQ_cb));
   if (*(queue_def->cb) == NULL)
   {
      return NULL;
   }
   (*(queue_def->cb))->queue_def = queue_def;

   /* Create a queue in FreeRTOS */
   (*(queue_def->cb))->handle = xQueueCreate(queue_def->queue_sz, sizeof(void *));
   if ((*(queue_def->cb))->handle == NULL)
   {
      vPortFree(*(queue_def->cb));
      return NULL;
   }

   /* Create a mail pool */
   (*(queue_def->cb))->pool = osPoolCreate(&pool_def);
   if ((*(queue_def->cb))->pool == NULL)
   {
      //TODO: Delete queue. How to do it in FreeRTOS?
      vPortFree(*(queue_def->cb));
      return NULL;
   }

   return *(queue_def->cb);
}

/**
* @brief Allocate a memory block from a mail
* @param  queue_id      mail queue ID obtained with \ref osMailCreate.
* @param  millisec      timeout value or 0 in case of no time-out.
* @retval pointer to memory block that can be filled with mail or NULL in case error.
* @note   MUST REMAIN UNCHANGED: \b osMailAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osMailAlloc (osMailQId queue_id, uint32_t millisec)
{
   (void) millisec;
   void *p;


   if (queue_id == NULL)
   {
      return NULL;
   }

   p = osPoolAlloc(queue_id->pool);

   return p;
}

/**
* @brief Allocate a memory block from a mail and set memory block to zero
* @param  queue_id      mail queue ID obtained with \ref osMailCreate.
* @param  millisec      timeout value or 0 in case of no time-out.
* @retval pointer to memory block that can be filled with mail or NULL in case error.
* @note   MUST REMAIN UNCHANGED: \b osMailCAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osMailCAlloc (osMailQId queue_id, uint32_t millisec)
{
   uint32_t i;
   void *p = osMailAlloc(queue_id, millisec);

   if (p)
   {
      for (i = 0; i < sizeof(queue_id->queue_def->item_sz); i++)
      {
         ((uint8_t *)p)[i] = 0;
      }
   }

   return p;
}

/**
* @brief Put a mail to a queue
* @param  queue_id      mail queue ID obtained with \ref osMailCreate.
* @param  mail          memory block previously allocated with \ref osMailAlloc or \ref osMailCAlloc.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMailPut shall be consistent in every CMSIS-RTOS.
*/
osStatus osMailPut (osMailQId queue_id, void *mail)
{
   portBASE_TYPE taskWoken;


   if (queue_id == NULL)
   {
      return osErrorParameter;
   }

   taskWoken = pdFALSE;

   if (inHandlerMode())
   {
      if (xQueueSendFromISR(queue_id->handle, &mail, &taskWoken) != pdTRUE)
      {
         return osErrorOS;
      }
      portEND_SWITCHING_ISR(taskWoken);
   }
   else
   {
      if (xQueueSend(queue_id->handle, &mail, 0) != pdTRUE)
      {
         return osErrorOS;
      }
   }

   return osOK;
}

/**
* @brief Get a mail from a queue
* @param  queue_id   mail queue ID obtained with \ref osMailCreate.
* @param millisec    timeout value or 0 in case of no time-out
* @retval event that contains mail information or error code.
* @note   MUST REMAIN UNCHANGED: \b osMailGet shall be consistent in every CMSIS-RTOS.
*/
osEvent osMailGet (osMailQId queue_id, uint32_t millisec)
{
   portBASE_TYPE taskWoken;
   portTickType ticks;
   osEvent event;

   event.def.mail_id = queue_id;

   if (queue_id == NULL)
   {
      event.status = osErrorParameter;
      return event;
   }

   taskWoken = pdFALSE;

   ticks = 0;
   if (millisec == osWaitForever)
   {
      ticks = portMAX_DELAY;
   }
   else if (millisec != 0)
   {
      ticks = millisec / portTICK_RATE_MS;
      if (ticks == 0)
      {
         ticks = 1;
      }
   }

   if (inHandlerMode())
   {
      if (xQueueReceiveFromISR(queue_id->handle, &event.value.p, &taskWoken) == pdTRUE)
      {
         /* We have mail */
         event.status = osEventMail;
      }
      else
      {
         event.status = osOK;
      }
      portEND_SWITCHING_ISR(taskWoken);
   }
   else
   {
      if (xQueueReceive(queue_id->handle, &event.value.p, ticks) == pdTRUE)
      {
         /* We have mail */
         event.status = osEventMail;
      }
      else
      {
         event.status = (ticks == 0) ? osOK : osEventTimeout;
      }
   }

   return event;
}

/**
* @brief Free a memory block from a mail
* @param  queue_id mail queue ID obtained with \ref osMailCreate.
* @param  mail     pointer to the memory block that was obtained with \ref osMailGet.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMailFree shall be consistent in every CMSIS-RTOS.
*/
osStatus osMailFree (osMailQId queue_id, void *mail)
{
   if (queue_id == NULL)
   {
      return osErrorParameter;
   }

   osPoolFree(queue_id->pool, mail);

   return osOK;
}
#endif  /* Use Mail Queues */


/*************************** Additional specific APIs to Free RTOS ************/
/**
* @brief  Suspend execution of a thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadSuspend (osThreadId thread_id)
{
   return SuspendThread(thread_id) != -1 ? osOK : osErrorOS;
}

/**
* @brief  Resume execution of a suspended thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadResume (osThreadId thread_id)
{
   return ResumeThread(thread_id) != -1 ? osOK : osErrorOS;
}

/**
* @brief  Suspend execution of a all active threads.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadSuspendAll (void)
{
   // Not impl.
   return osErrorOS;
}

/**
* @brief  Resume execution of a all suspended threads.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadResumeAll (void)
{
   // Not impl.
   return osErrorOS;
}

/**
* @brief  Check if a thread is already suspended or not.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadIsSuspended(osThreadId thread_id)
{
#if (INCLUDE_vTaskSuspend == 1)
   if (xTaskIsTaskSuspended(thread_id) != pdFALSE)
      return osOK;
   else
      return osErrorOS;
#else
   return osErrorResource;
#endif
}

/**
* @brief  Delay a task until a specified time
* @param   PreviousWakeTime   Pointer to a variable that holds the time at which the
*          task was last unblocked.
* @param   millisec    time delay value
* @retval  status code that indicates the execution status of the function.
*/
osStatus osDelayUntil (uint32_t PreviousWakeTime, uint32_t millisec)
{
   return osErrorOS;
}

/**
* @brief   Lists all the current threads, along with their current state
*          and stack usage high water mark.
* @param   buffer   A buffer into which the above mentioned details
*          will be written
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadList (int8_t *buffer)
{
   return osErrorOS;
}

/**
* @brief  Create and Initialize a Recursive Mutex
* @param  mutex_def     mutex definition referenced with \ref osMutex.
* @retval  mutex ID for reference by other functions or NULL in case of error..
*/
osMutexId osRecursiveMutexCreate (osMutexDef_t *mutex_def);

/**
* @brief  Release a Recursive Mutex
* @param   mutex_id      mutex ID obtained by \ref osRecursiveMutexCreate.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osRecursiveMutexRelease (osMutexId mutex_id);

/**
* @brief  Release a Recursive Mutex
* @param   mutex_id    mutex ID obtained by \ref osRecursiveMutexCreate.
* @param millisec      timeout value or 0 in case of no time-out.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osRecursiveMutexWait (osMutexId mutex_id, uint32_t millisec);



