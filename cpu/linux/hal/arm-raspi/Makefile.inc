
CROSSCOMPILER=/opt/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CFLAGS += -std=gnu11
CFLAGS += -DCPU_ARM_RASPI
PROJECT_CPU_DIRS += hal/arm-raspi
PROJECT_SOURCEFILES += hal_gpio.c

PROJECT_CPU_DIRS += hal/arm-raspi/drivers/bcm2835
PROJECT_SOURCEFILES += bcm2835.c
