
#include <stdlib.h>
#include <malloc.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/*-----------------------------------------------------------*/

extern void vApplicationMallocFailedHook( size_t size );

extern char _end;      // end of bss section, defined in linker script 
extern char _eheap;    // end of heap, defined in linker script 
#define HEAP_SIZE (&_eheap - &_end)

static unsigned int min_heap_size = configTOTAL_HEAP_SIZE;

void __malloc_lock(struct _reent *_r)
{  
   if (!vPortInHandlerMode())
      vTaskSuspendAll();
}

void __malloc_unlock(struct _reent *_r)
{
   if (!vPortInHandlerMode())
      xTaskResumeAll();
}

void *pvPortMalloc(size_t xWantedSize)
{
   void *pvReturn;
   struct mallinfo mi;

#if (configUSE_MALLOC_CHECK_FREE_MEM == 1)	
   mi = mallinfo();	
	if (HEAP_SIZE - mi.uordblks < min_heap_size)
      min_heap_size = HEAP_SIZE - mi.uordblks;
#endif
	pvReturn = malloc(xWantedSize);

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if (pvReturn == NULL)
			vApplicationMallocFailedHook(xWantedSize);
	}
	#endif

	return pvReturn;
}

void *pvPortRealloc(void *ptr, size_t size)
{
   void *pvReturn;

	pvReturn = realloc(ptr, size);

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if (pvReturn == NULL)
			vApplicationMallocFailedHook(size);
	}
	#endif

	return pvReturn;
}

void vPortFree(void *pv)
{
	if( pv )
	{
      free(pv);
		traceFREE(pv, 0);
	}
}

unsigned int xPortGetFreeHeapSize(void)
{
   struct mallinfo mi;
   mi = mallinfo();
   return HEAP_SIZE - mi.uordblks;
}

unsigned int xPortGetTotalHeapSize(void)
{
    return HEAP_SIZE;
}

unsigned int xPortGetMinHeapSize(void)
{
    return min_heap_size;
}

