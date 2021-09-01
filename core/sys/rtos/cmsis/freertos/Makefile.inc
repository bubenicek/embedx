
PROJECT_DIRS += $(EMBEDX_ROOT)/core/sys/rtos/cmsis/freertos

CFLAGS += -DCFG_CMSIS_OS_API=1

PROJECT_SOURCEFILES += cmsis_os.c

