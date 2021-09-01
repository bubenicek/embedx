
PROJECT_CPU = ../../cpu/stm8

PROJECT_CPU_DIRS = . hal

PROJECT_SOURCEFILES += hal_console.c
PROJECT_SOURCEFILES += hal_time.c
PROJECT_SOURCEFILES += hal_gpio.c
PROJECT_SOURCEFILES += hal_spi.c
PROJECT_SOURCEFILES += hal_uart.c
PROJECT_SOURCEFILES += hal_wdg.c
PROJECT_SOURCEFILES += hal_pm.c


### Compiler definitions
CC       = sdcc
LD       = sdld

CFLAGS += -lstm8 -mstm8

LDFLAGS += 

### Setup directory search path for source files

CUSTOM_RULE_C_TO_O=yes
%.o: %.c
	$(CC) $(CFLAGS) $< 

CUSTOM_RULE_C_TO_OBJECTDIR_O=yes
$(OBJECTDIR)/%.o: %.c | $(OBJECTDIR)
	$(CC) $(CFLAGS) -$< 


CUSTOM_RULE_LINK=yes
%.o: %.c
#       $(CC) $(CFLAGS) -c $< -o $@
#        $(STRIP) --strip-unneeded -g -x $@

