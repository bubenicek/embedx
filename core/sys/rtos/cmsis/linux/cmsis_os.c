
#include "system.h"

#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <errno.h>
#include <sched.h>

#include "cmsis_os.h"

TRACE_TAG(cmsis_os);


typedef void *(*pthread_start_routine_t) (void *);


/*********************** Kernel Control Functions *****************************/

/// Initialize the RTOS Kernel for creating objects.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osKernelInitialize shall be consistent in every CMSIS-RTOS.
osStatus osKernelInitialize (void)
{
   osTimerKernelInit();
   return osOK;
}


/**
* @brief  Start the RTOS Kernel with executing the specified thread.
* @param  thread_def    thread definition referenced with \ref osThread.
* @param  argument      pointer that is passed to the thread function as start argument.
* @retval status code that indicates the execution status of the function
* @note   MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
*/
osStatus osKernelStart (void)
{
   pthread_mutex_t mutex;
   pthread_cond_t cond;

   // Suspend main thread
   pthread_mutex_init(&mutex, 0);
   pthread_cond_init(&cond, 0);

   pthread_mutex_lock(&mutex);
   pthread_cond_wait(&cond, &mutex);
   pthread_mutex_unlock(&mutex);

//   osTimerKernelHandler();

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
    static uint32_t start_tm = 0;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    if (start_tm == 0)
    {
        start_tm = 1000 * tp.tv_sec + tp.tv_nsec / 1000000;
    }

    return (1000 * tp.tv_sec + tp.tv_nsec / 1000000) - start_tm;
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
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
	pthread_t tid;
   pthread_attr_t attr;

   // set thread detachstate attribute to DETACHED
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

   if (pthread_create(&tid, &attr, (pthread_start_routine_t)thread_def->pthread, argument) != 0)
      return 0;

   return tid;
}


/**
* @brief  Return the thread ID of the current running thread.
* @retval thread ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
*/
osThreadId osThreadGetId (void)
{
   return pthread_self();
}


/**
* @brief  Terminate execution of a thread and remove it from Active Threads.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadTerminate (osThreadId thread_id)
{
   return pthread_cancel(thread_id) == 0 ? osOK : osErrorOS;
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
   return 0;
}

/**
* @brief   Get current priority of an active thread.
* @param   thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  current priority value of the thread function.
* @note   MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
*/
osPriority osThreadGetPriority (osThreadId thread_id)
{
   return osPriorityNormal;
}

/*********************** Generic Wait Functions *******************************/
/**
* @brief   Wait for Timeout (Time Delay)
* @param   millisec      time delay value
* @retval  status code that indicates the execution status of the function.
*/
osStatus osDelay (uint32_t millisec)
{
   usleep(millisec * 1000);
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
osMutexId osMutexCreate (const osMutexDef_t *mutex_def)
{
   pthread_mutex_t *mtx = NULL;

   if ((mtx = osMemAlloc(sizeof(pthread_mutex_t))) != NULL)
   {
      if (pthread_mutex_init(mtx, NULL) != 0)
      {
         osMemFree(mtx);
         return NULL;
      }
   }

   return mtx;
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
   if (millisec == osWaitForever)
   {
      return pthread_mutex_lock(mutex_id) == 0 ? osOK : osErrorOS;
   }
   else
   {
      struct timespec wait;
      wait.tv_sec = 0;
      wait.tv_nsec = millisec * 1000 * 1000;

      return pthread_mutex_timedlock(mutex_id, &wait) == 0 ? osOK : osErrorTimeoutResource;
   }
}

/**
* @brief Release a Mutex that was obtained by \ref osMutexWait
* @param mutex_id      mutex ID obtained by \ref osMutexCreate.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexRelease (osMutexId mutex_id)
{
   return pthread_mutex_unlock(mutex_id) == 0 ? osOK : osErrorOS;
}

/**
* @brief Delete a Mutex
* @param mutex_id  mutex ID obtained by \ref osMutexCreate.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexDelete (osMutexId mutex_id)
{
   int res;

   res = pthread_mutex_destroy(mutex_id);
   osMemFree(mutex_id);

   return res == 0 ? osOK : osErrorOS;
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
osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count)
{
   sem_t *sem = NULL;

   if ((sem = osMemAlloc(sizeof(sem_t))) != NULL)
   {
      if (sem_init(sem, 0, count) != 0)
      {
         osMemFree(sem);
         return NULL;
      }
   }

   return sem;
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
   if (millisec == osWaitForever)
   {
      return sem_wait(semaphore_id) == 0 ? osOK : osErrorOS;
   }
   else
   {
      int res;
      struct timespec abstimeout;

      if (clock_gettime(CLOCK_REALTIME, &abstimeout) == -1)
      {
         TRACE_ERROR("clock_gettime");
         return osErrorOS;
      }

      abstimeout.tv_sec += millisec / 1000;
      abstimeout.tv_nsec += (millisec % 1000) * 1000000;
      if (abstimeout.tv_nsec >= 1000000000)
      {
         abstimeout.tv_sec += 1;
         abstimeout.tv_nsec -= 1000000000;
      }

      while ((res = sem_timedwait(semaphore_id, &abstimeout)) == -1 && errno == EINTR)
         continue;       // Restart if interrupted by handler

      if (res < 0)
      {
         return (errno == ETIMEDOUT) ? osErrorResource : osErrorOS;
      }
      else
      {
         return osOK;
      }
   }
}

/**
* @brief Release a Semaphore token
* @param  semaphore_id  semaphore object referenced with \ref osSemaphore.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id)
{
   return sem_post(semaphore_id) == 0 ? osOK : osErrorOS;
}

/**
* @brief Delete a Semaphore
* @param  semaphore_id  semaphore object referenced with \ref osSemaphore.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{
   int res;

   res = sem_destroy(semaphore_id);
   osMemFree(semaphore_id);

   return res == 0 ? osOK : osErrorOS;
}

#endif    /* Use Semaphores */

/*******************   Memory Pool Management Functions  ***********************/

#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))

typedef struct os_pool_cb
{
   void *pool;
   uint8_t *markers;
   uint32_t pool_sz;
   uint32_t item_sz;
   uint32_t currentIndex;
   osMutexId mutex;
} os_pool_cb_t;


/**
* @brief Create and Initialize a memory pool
* @param  pool_def      memory pool definition referenced with \ref osPool.
* @retval  memory pool ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osPoolCreate shall be consistent in every CMSIS-RTOS.
*/
osPoolId osPoolCreate (const osPoolDef_t *pool_def)
{
   osPoolId thePool;
   int itemSize = 4 * ((pool_def->item_sz + 3) / 4);
   uint32_t i;

   /* First have to allocate memory for the pool control block. */
   thePool = osMemAlloc(sizeof(os_pool_cb_t));
   if (thePool)
   {
      if ((thePool->mutex = osMutexCreate(NULL)) == NULL)
         return NULL;

      thePool->pool_sz = pool_def->pool_sz;
      thePool->item_sz = itemSize;
      thePool->currentIndex = 0;

      /* Memory for markers */
      thePool->markers = osMemAlloc(pool_def->pool_sz);
      if (thePool->markers)
      {
         /* Now allocate the pool itself. */
         thePool->pool = osMemAlloc(pool_def->pool_sz * itemSize);

         if (thePool->pool)
         {
            for (i = 0; i < pool_def->pool_sz; i++)
            {
               thePool->markers[i] = 0;
            }
         }
         else
         {
            osMutexDelete(thePool->mutex);
            osMemFree(thePool->markers);
            osMemFree(thePool);
            thePool = NULL;
         }
      }
      else
      {
         osMutexDelete(thePool->mutex);
         osMemFree(thePool);
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
   void *p = NULL;
   uintptr_t i;
   uintptr_t index;

   osMutexWait(pool_id->mutex, osWaitForever);

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
         p = (void *)((uintptr_t)(pool_id->pool) + (index * pool_id->item_sz));
         pool_id->currentIndex = index;
         break;
      }
   }

   osMutexRelease(pool_id->mutex);

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

   osMutexWait(pool_id->mutex, osWaitForever);

   if (pool_id == NULL)
      throw_exception(fail);

   if (block == NULL)
      throw_exception(fail);

   if (block < pool_id->pool)
      throw_exception(fail);

   index = (uintptr_t)block - (uintptr_t)(pool_id->pool);
   if (index % pool_id->item_sz)
      throw_exception(fail);

   index = index / pool_id->item_sz;
   if (index >= pool_id->pool_sz)
      throw_exception(fail);

   pool_id->markers[index] = 0;

   osMutexRelease(pool_id->mutex);

   return osOK;

fail:
   osMutexRelease(pool_id->mutex);
   return osErrorParameter;
}

#endif   /* Use Memory Pool Management */

/*******************   Message Queue Management Functions  *********************/

#if (defined (osFeature_MessageQ)  &&  (osFeature_MessageQ != 0)) /* Use Message Queues */

typedef struct
{
   pthread_mutex_t mutex;
   pthread_cond_t cond;
   uint16_t size;
   uint16_t head;
   uint16_t tail;
   uintptr_t data[1];

} message_queue_t;


/**
* @brief Create and Initialize a Message Queue
* @param queue_def     queue definition referenced with \ref osMessageQ.
* @param  thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
* @retval  message queue ID for reference by other functions or NULL in case of error.
* @note   MUST REMAIN UNCHANGED: \b osMessageCreate shall be consistent in every CMSIS-RTOS.
*/
osMessageQId osMessageCreate(const osMessageQDef_t *queue_def, osThreadId thread_id)
{
   message_queue_t *queue = NULL;

   queue = osMemAlloc(sizeof(message_queue_t) + queue_def->queue_sz * sizeof(uintptr_t));
   if (queue == NULL)
      throw_exception(fail);

   if (pthread_cond_init(&queue->cond, NULL) != 0)
      throw_exception(fail_cond);

   if (pthread_mutex_init(&queue->mutex, NULL) != 0)
      throw_exception(fail_mutex);

   memset(queue, 0, sizeof(message_queue_t) + queue_def->queue_sz * sizeof(uintptr_t));
   queue->size = queue_def->queue_sz;
   queue->tail = queue->head = 0;

   return queue;

fail_mutex:
   pthread_cond_destroy(&queue->cond);
fail_cond:
   osMemFree(queue);
fail:
   return NULL;
}

/// Destroy message queue
osStatus osMessageDestroy(osMessageQId queue_id)
{
   message_queue_t *queue = queue_id;

   pthread_cond_destroy(&queue->cond);
   pthread_mutex_destroy(&queue->mutex);

   osMemFree(queue);

   return osOK;
}


/**
* @brief Put a Message to a Queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  info      message information.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMessagePut shall be consistent in every CMSIS-RTOS.
*/
osStatus osMessagePut (osMessageQId queue_id, uintptr_t info, uint32_t millisec)
{
   message_queue_t *queue = queue_id;
   uint16_t nxthead;

   pthread_mutex_lock(&queue->mutex);

   nxthead = (queue->head + 1) & (queue->size - 1);
   if (nxthead == queue->tail)
   {
      // Queue is full
      pthread_mutex_unlock(&queue->mutex);
      return osErrorNoMemory;
   }

   if (queue->head == queue->tail)
      pthread_cond_broadcast(&queue->cond);

   queue->data[queue->head] = info;
   queue->head = nxthead;

   pthread_mutex_unlock(&queue->mutex);

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
   int res = 0;
   struct timespec abstimeout;
   message_queue_t *queue = queue_id;
   osEvent event;

   event.def.message_id = queue_id;
   if (queue_id == NULL)
   {
      event.status = osErrorParameter;
      return event;
   }

   if (millisec != osWaitForever)
   {
      struct timeval now;

      gettimeofday(&now, NULL);
      abstimeout.tv_sec = now.tv_sec;
      abstimeout.tv_nsec = (now.tv_usec * 1000) + (millisec * 1000 * 1000);
      if (abstimeout.tv_nsec >= 1000000000)
      {
         abstimeout.tv_sec++;
         abstimeout.tv_nsec -= 1000000000;
      }
   }

   pthread_mutex_lock(&queue->mutex);

   // Will wait until awakened by a signal or broadcast
   while (queue->head == queue->tail && res != ETIMEDOUT)
   {
      // Need to loop to handle spurious wakeups
      if (millisec != osWaitForever)
         res = pthread_cond_timedwait(&queue->cond, &queue->mutex, &abstimeout);
      else
         res = pthread_cond_wait(&queue->cond, &queue->mutex);
    }

   if (res == ETIMEDOUT)
   {
      pthread_mutex_unlock(&queue->mutex);
      event.status = osEventTimeout;
      return event;
   }

   event.status = osEventMessage;
   event.value.v = queue->data[queue->tail];
   queue->tail = (queue->tail + 1) & (queue->size -1);

   pthread_mutex_unlock(&queue->mutex);

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
   osEvent event = {0};
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
   *(queue_def->cb) = osMemAlloc(sizeof(struct os_mailQ_cb));
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
      osMemFree(*(queue_def->cb));
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
   // Not impl.
   return osErrorOS;
}

/**
* @brief  Resume execution of a suspended thread.
* @param   thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
* @retval  status code that indicates the execution status of the function.
*/
osStatus osThreadResume (osThreadId thread_id)
{
    // Not impl.
   return osErrorOS;
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
osStatus osThreadList (uint8_t *buffer)
{
   return osErrorOS;
}

/**
* @brief  Create and Initialize a Recursive Mutex
* @param  mutex_def     mutex definition referenced with \ref osMutex.
* @retval  mutex ID for reference by other functions or NULL in case of error..
*/
osMutexId osRecursiveMutexCreate (const osMutexDef_t *mutex_def);

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


unsigned int osMemGetFreeSize(void)
{
   struct mallinfo mi;
   mi = mallinfo();
   return mi.fordblks;
}

unsigned int osMemGetTotalSize(void)
{
   struct mallinfo mi;
   mi = mallinfo();
   return mi.uordblks + mi.fordblks;
}

unsigned int osMemGetMinSize(void)
{
    return 0;
}


//====================================
//==== Asynchronous function call ====
//====================================

#ifndef CFG_ASYNC_CALL_QUEUE_SIZE
#define CFG_ASYNC_CALL_QUEUE_SIZE       16
#endif

// Types:
typedef struct
{
    osAsyncCallFunction func;
    void *arg;

} async_call_t;

// Prototypes:
static void async_call_thread(void *arg);

const osMessageQDef(ASYNC_CALL_QUEUE, CFG_ASYNC_CALL_QUEUE_SIZE, uint32_t);
static const osThreadDef(ASYNC_CALL, async_call_thread, 0, 0, 4096);

static osMessageQId asyncCallQueueId = NULL;
static osThreadId asyncCallThreadId = 0;

/** Asynchronous function executing */
osStatus osAsyncCall(osAsyncCallFunction func, void *arg)
{
    async_call_t *call;

    if (!asyncCallThreadId)
    {
        if ((asyncCallQueueId = osMessageCreate(osMessageQ(ASYNC_CALL_QUEUE), osThreadGetId())) == NULL)
        {
            TRACE_ERROR("Create async call queue");
            throw_exception(fail);
        }

        // Start monitor thread
        if ((asyncCallThreadId = osThreadCreate(osThread(ASYNC_CALL), NULL)) == 0)
        {
            osMessageDestroy(asyncCallQueueId);
            asyncCallQueueId = NULL;
            TRACE_ERROR("Start async call thread failed");
            throw_exception(fail);
        }

        TRACE("Async call initialized");
    }

    if ((call = osMemAlloc(sizeof(async_call_t))) == NULL)
    {
        TRACE_ERROR("Alloc async call item queue failed");
        throw_exception(fail);
    }

    call->func = func;
    call->arg = arg;

    if (osMessagePut(asyncCallQueueId, (uintptr_t)call, 1) != osOK)
    {
        TRACE_ERROR("Add sync call item into queue failed");
        osMemFree(call);
        throw_exception(fail);
    }

    return osOK;

fail:
    return osErrorOS;
}

static void async_call_thread(void *arg)
{
    osEvent evt;
    async_call_t *call;

    TRACE("Async call thread is running ...");

    while (1)
    {
        evt = osMessageGet(asyncCallQueueId, osWaitForever);
        if (evt.status != osEventMessage)
        {
            TRACE_ERROR("Bad async call item");
            continue;
        }

        call = evt.value.p; 
        
        if (call->func != NULL)       
            call->func(call->arg);
        
        osMemFree(call);
    }
}