
PROJECT_DIRS += $(EMBEDX_ROOT)/core/sys/rtos 
PROJECT_DIRS += $(EMBEDX_ROOT)/core/sys/rtos/freertos 
PROJECT_DIRS += $(EMBEDX_ROOT)/core/sys/rtos/freertos/portable/MemMang

ifeq ($(ARM),ARMCM4_FP)
   PROJECT_DIRS += $(EMBEDX_ROOT)/core/sys/rtos/freertos/portable/GCC/ARM_CM4F/
else
   PROJECT_DIRS += $(EMBEDX_ROOT)/core/sys/rtos/freertos/portable/GCC/ARM_CM3/
endif

PROJECT_SOURCEFILES += rtos_hook.c
PROJECT_SOURCEFILES += rtos_list.c
PROJECT_SOURCEFILES += queue.c
PROJECT_SOURCEFILES += tasks.c
PROJECT_SOURCEFILES += timers.c
PROJECT_SOURCEFILES += port.c
PROJECT_SOURCEFILES += heap_3.c


