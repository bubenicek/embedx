
PROJECT_DIRS += $(EMBEDX_ROOT)/cpu/stm32f4/lwip 
PROJECT_DIRS += $(EMBEDX_ROOT)/cpu/stm32f4/lwip/FreeRTOS

PROJECT_SOURCEFILES += ethernetif.c
PROJECT_SOURCEFILES += sys_arch.c

CFLAGS += -DCFG_USE_LWIP

include $(EMBEDX_ROOT)/core/net/lwip/Makefile.inc


