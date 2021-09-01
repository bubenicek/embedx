
PROJECT = test-rtos-os

PROJECT_DIRS += src 
PROJECT_SOURCEFILES += main.c


ifeq ($(LINKMODE),boot)
all: $(PROJECT) makebin makebin_boot
else
all: $(PROJECT) 
endif

### Compiler options
CFLAGS += -O2 -g
CFLAGS += -DCFG_DEBUG=1

### RTOS 
include ../../core/sys/rtos/Makefile.inc

### Filesystem
#include ../../core/fs/spiffs/Makefile.inc
#include ../../core/fs/fatfs/Makefile.inc

### Common sources
include ../../Makefile.inc

makebin:
	$(Q)$(OBJCOPY) -O binary $(PROJECT).$(TARGET) $(PROJECT).$(TARGET).bin
	$(OBJDUMP) -D -S $(PROJECT).$(TARGET) > $(PROJECT).$(TARGET).lst
#	$(Q)$(FWCRC) $(PROJECT).$(TARGET).bin $(CFG_APP_HEADER_OFFSET)
	$(Q)$(SIZE) -A $(PROJECT).$(TARGET)
	$(Q)$(SIZE) $(PROJECT).$(TARGET)

makebin_boot:
	$(Q)cp ../bootloader/bootloader.$(TARGET).bin boot.$(PROJECT).$(TARGET).bin
	$(Q)cat $(PROJECT).$(TARGET).bin >> boot.$(PROJECT).$(TARGET).bin
	
