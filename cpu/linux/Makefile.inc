
PROJECT_CPU = $(EMBEDX_ROOT)/cpu/linux
PROJECT_CPU_DIRS = . hal

# Common HAL sources
PROJECT_SOURCEFILES += hal_console.c
PROJECT_SOURCEFILES += hal_time.c
PROJECT_SOURCEFILES += hal_uart.c
PROJECT_SOURCEFILES += hal_uart_baudrate.c
PROJECT_SOURCEFILES += hal_dmx.c
PROJECT_SOURCEFILES += hal_pm.c
PROJECT_SOURCEFILES += hal_rtc.c
PROJECT_SOURCEFILES += hal_net.c
PROJECT_SOURCEFILES += hal_uuid.c
PROJECT_SOURCEFILES += hal_xmem.c
PROJECT_SOURCEFILES += hal_spi.c

## CPU depends
ifeq ($(CPU),x86)
	include $(PROJECT_CPU)/hal/x86/Makefile.inc
else ifeq ($(CPU),ARM_H3)
	include $(PROJECT_CPU)/hal/arm-h3/Makefile.inc
else ifeq ($(CPU),ARM_A13)
	include $(PROJECT_CPU)/hal/arm-a13/Makefile.inc
else ifeq ($(CPU),ARM_A20)
	include $(PROJECT_CPU)/hal/arm-a20/Makefile.inc
else ifeq ($(CPU),ARM_RASPI)
	include $(PROJECT_CPU)/hal/arm-raspi/Makefile.inc
else
	$(error Not supported linux CPU)
endif


### Compiler definitions
CC       = $(CROSSCOMPILER)gcc
LD       = $(CROSSCOMPILER)gcc
AS       = $(CROSSCOMPILER)as
OBJCOPY  = $(CROSSCOMPILER)objcopy
STRIP    = $(CROSSCOMPILER)strip
OBJDUMP	= $(CROSSCOMPILER)objdump
SIZE 		= $(CROSSCOMPILER)size


#CFLAGS += -Wall -I/usr/local/include
#CFLAGS += -ffunction-sections
#CFLAGS += -fdata-sections
#CFLAGS += -fno-strict-aliasing
#CFLAGS += -fPIC

#LDFLAGS += -static
#LDFLAGS += -ffunction-sections
#LDFLAGS += -fdata-sections
#LDFLAGS += -Wl,-Map=$(PROJECT_TARGET).map,-export-dynamic
LIBS += -lpthread -lrt -lm

### Compilation rules
makebin:
	$(Q)$(SIZE) -A $(PROJECT_TARGET)
	$(Q)$(SIZE) $(PROJECT_TARGET)

load:
	$(STRIP) $(PROJECT_TARGET)
	sshpass -p pi scp $(PROJECT_TARGET) pi@10.0.0.130:~/
