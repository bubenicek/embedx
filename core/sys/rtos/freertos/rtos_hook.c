
#include "system.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

TRACE_TAG(freertos_hook);

//----------------------------------------------------------------------------------------------
//                                      OpenRTOS callbacks
//----------------------------------------------------------------------------------------------

/**
 * @brief This function will get called if a task overflows its stack.
 *        If the parameters are corrupt then inspect pxCurrentTCB to find which was the offending task.
 */
void vApplicationStackOverflowHook ( xTaskHandle * pxTask, signed char * pcTaskName )
{
   TRACE_ERROR("STACK OWERFLOW '%s'", pcTaskName);
   while(1);
}


/**
 * @brief This function will get called if pvPortMalloc function returns NULL.
 */
void vApplicationMallocFailedHook( size_t xWantedSize )
{
    TRACE_ERROR("MALLOC FAILED, wanted_size=%d   free_size=%d", xWantedSize, xPortGetFreeHeapSize());
}

