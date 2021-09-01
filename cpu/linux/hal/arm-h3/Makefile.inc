
CROSSCOMPILER=/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-
CFLAGS += -std=gnu11
CFLAGS += -DCPU_ARM_H3
PROJECT_CPU_DIRS += hal/arm-h3
PROJECT_SOURCEFILES += hal_gpio.c
PROJECT_SOURCEFILES += hal_i2c.c

PROJECT_CPU_DIRS += hal/arm-h3/drivers/wiringPi
PROJECT_SOURCEFILES += boardtype_friendlyelec.c
PROJECT_SOURCEFILES += wiringPi.c
