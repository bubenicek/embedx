
PROJECT_CPU = $(EMBEDX_ROOT)/cpu/avr

PROJECT_CPU_DIRS = . drivers hal

# Drivers
PROJECT_SOURCEFILES += sw_uart.c

# HAL
PROJECT_SOURCEFILES += hal_console.c
PROJECT_SOURCEFILES += hal_time.c
PROJECT_SOURCEFILES += hal_gpio.c
PROJECT_SOURCEFILES += hal_spi.c
PROJECT_SOURCEFILES += hal_uart.c
PROJECT_SOURCEFILES += hal_wdg.c
PROJECT_SOURCEFILES += hal_pm.c


### Compiler definitions
CC       = avr-gcc
LD       = avr-gcc
AS       = avr-as
OBJCOPY  = avr-objcopy
OBJDUMP  = avr-objdump
STRIP    = avr-strip
SIZE	 	= avr-size

CFLAGS += -mmcu=atmega328p
CFLAGS += -DF_CPU=16000000UL
CFLAGS += -ffunction-sections

LDFLAGS += -L/usr/lib
LDFLAGS += -mmcu=atmega328p
LDFLAGS += -ffunction-sections
LDFLAGS += -Wl,-Map=$(PROJECT).$(TARGET).map,--cref
LDFLAGS += -Wl,--gc-sections


### Rules
load:
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 57600 -U flash:w:$(PROJECT).$(TARGET).hex
