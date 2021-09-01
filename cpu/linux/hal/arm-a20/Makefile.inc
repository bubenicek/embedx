CROSSCOMPILER=/opt/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CFLAGS += -DCPU_ARM_A20
PROJECT_CPU_DIRS += hal/arm-a20 hal/arm-a20/drivers
PROJECT_SOURCEFILES += sunxi_gpio.c
PROJECT_SOURCEFILES += hal_gpio.c
PROJECT_SOURCEFILES += hal_i2c.c
