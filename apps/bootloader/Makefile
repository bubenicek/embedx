
PROJECT = bootloader

PROJECT_DIRS += src src/cfg 
PROJECT_SOURCEFILES += main.c

all: $(PROJECT) makebin

### Compiler options
CFLAGS += -Os

### Filesystem
include ../../core/fs/uffs/platform/standalone/Makefile.inc

include ../../Makefile.inc

makebin:
	$(Q)$(OBJCOPY) -O binary $(PROJECT).$(TARGET) $(PROJECT).$(TARGET).bin
	$(Q)$(FWCRC) $(PROJECT).$(TARGET).bin $(CFG_APP_HEADER_OFFSET)
	$(Q)$(OBJCOPY) -I binary -O binary --pad-to=$(CFG_BOOTLOADER_SIZE) --gap-fill=0xFF $(PROJECT).$(TARGET).bin
	$(Q)$(SIZE) -A $(PROJECT).$(TARGET)
	$(Q)$(SIZE) $(PROJECT).$(TARGET)
