
CROSSCOMPILER=
CFLAGS += -DCPU_X86
CFLAGS += -Wno-format-truncation
PROJECT_CPU_DIRS += hal/x86
PROJECT_SOURCEFILES += hal_gpio.c

LIBS += -lm
