
#include "system.h"

#define TRACE_TAG "Main"
#if !ENABLE_TRACE_MAIN
#undef TRACE
#define TRACE(...)
#endif

#if 0

// Prototypes:
static void timer_cb(const void *arg);

typedef struct 
{
    float    voltage;   /* AD result of measured voltage */
    float    current;   /* AD result of measured current */
    uint32_t counter;   /* A counter value               */

} message_t;


osPoolDef(mpool, 4, message_t);
osPoolId  mpool;

const osMessageQDef(CMD_Q, 16, uint32_t);
osMessageQId cmd_q;

const osTimerDef(TMR, timer_cb);
osTimerId timer;

osMutexId mutex_id;

static void timer_cb(const void *arg)
{
   TRACE("timer_cb");
}

/** System initialization thread */
static void system_init_thread(const void *arg)
{
   uint32_t i = 0;
 
   while(1)
   {
      VERIFY(osMutexWait(mutex_id, osWaitForever) == osOK);
      
      i++; // fake data update
      message_t *message = (message_t*)osPoolAlloc(mpool);
      ASSERT(message != NULL);
      
      TRACE("msg: %p", message);
      
      message->voltage = (i * 0.1) * 33; 
      message->current = (i * 0.1) * 11;
      message->counter = i;
      osMessagePut(cmd_q, (uintptr_t)message, osWaitForever);
      osDelay(1000);  
      TRACE("Send msg: %d", i);
      
      VERIFY(osMutexRelease(mutex_id) == osOK);
   }
   
   // Terminate init thread
   VERIFY(osThreadTerminate(osThreadGetId()) == osOK);
}


static void binding_thread(const void *arg)
{
   osEvent e;
   
   TRACE("Waiting for commands ...");
   
   while(1)
   {
      e = osMessageGet(cmd_q, 0);
      if (e.status == osEventMessage)
      {
            message_t *message = e.value.p;
            TRACE("Voltage: %.2f V", message->voltage);
            TRACE("Current: %.2f A", message->current);
            TRACE("Number of cycles: %u\n", message->counter);
            
            osPoolFree(mpool, message);                  
      }
      else
      {
         TRACE_ERROR("Get message failed");
      }
   }   
}

int main(void)
{   
   const osThreadDef(INIT, system_init_thread, osPriorityNormal, 0, 4096);   
   const osThreadDef(BIND, binding_thread, osPriorityNormal, 0, 4096);   
   
   // Initialize HW board
   VERIFY_FATAL(board_init() == 0);
   
   mutex_id = osMutexCreate(NULL);
   VERIFY_FATAL(mutex_id != NULL);
   
   cmd_q = osMessageCreate(osMessageQ(CMD_Q), osThreadGetId());
   VERIFY_FATAL(cmd_q != NULL);
   
   mpool = osPoolCreate(osPool(mpool));
   VERIFY_FATAL(mpool != NULL);
   
   timer = osTimerCreate(osTimer(TMR), osTimerPeriodic, NULL);
   VERIFY_FATAL(timer != NULL);
   VERIFY(osTimerStart(timer, 500) == osOK);
   
   // Start init thread
   osThreadCreate(osThread(INIT), NULL);
   osThreadCreate(osThread(BIND), NULL);
   
   // Start scheduler
   osKernelStart();

   return 0;
}

#endif

int main(void)
{
   return 0;
}
