
PROJECT_CPU = $(EMBEDX_ROOT)/cpu/stm32f4
PROJECT_CPU_DIRS = . hal cmsis/include drivers/include drivers/src drivers/libtm

PROJECT_SOURCEFILES += compiler.c

PROJECT_SOURCEFILES += misc.c
PROJECT_SOURCEFILES += stm32f4x7_eth.c
PROJECT_SOURCEFILES += stm32f4xx_exti.c
PROJECT_SOURCEFILES += stm32f4xx_flash.c
PROJECT_SOURCEFILES += stm32f4xx_gpio.c
PROJECT_SOURCEFILES += stm32f4xx_iwdg.c
PROJECT_SOURCEFILES += stm32f4xx_pwr.c
PROJECT_SOURCEFILES += stm32f4xx_rcc.c
PROJECT_SOURCEFILES += stm32f4xx_rng.c
PROJECT_SOURCEFILES += stm32f4xx_rtc.c
PROJECT_SOURCEFILES += stm32f4xx_spi.c
PROJECT_SOURCEFILES += stm32f4xx_syscfg.c
PROJECT_SOURCEFILES += stm32f4xx_tim.c
PROJECT_SOURCEFILES += stm32f4xx_usart.c
PROJECT_SOURCEFILES += stm32f4xx_wwdg.c
PROJECT_SOURCEFILES += stm32f4xx_swo.c
PROJECT_SOURCEFILES += stm32f4xx_dma.c
PROJECT_SOURCEFILES += stm32f4xx_dac.c
PROJECT_SOURCEFILES += stm32f4xx_adc.c

PROJECT_SOURCEFILES += tm_stm32f4_dma.c
PROJECT_SOURCEFILES += tm_stm32f4_spi_dma.c
PROJECT_SOURCEFILES += tm_stm32f4_gpio.c
PROJECT_SOURCEFILES += tm_stm32f4_adc.c
PROJECT_SOURCEFILES += tm_stm32f4_timer_properties.c
PROJECT_SOURCEFILES += tm_stm32f4_dac_signal.c
#PROJECT_SOURCEFILES += tm_stm32f4_fft.c

# HAL
PROJECT_SOURCEFILES += hal_console.c
PROJECT_SOURCEFILES += hal_flash.c
PROJECT_SOURCEFILES += hal_gpio.c
PROJECT_SOURCEFILES += hal_adc.c
PROJECT_SOURCEFILES += hal_pm.c
PROJECT_SOURCEFILES += hal_rand.c
PROJECT_SOURCEFILES += hal_rtc.c
PROJECT_SOURCEFILES += hal_spi.c
PROJECT_SOURCEFILES += hal_time.c
PROJECT_SOURCEFILES += hal_uart.c
PROJECT_SOURCEFILES += hal_uuid.c
PROJECT_SOURCEFILES += hal_wdg.c
PROJECT_SOURCEFILES += hal_net.c


### Compiler definitions
#CROSS_FW_ARM = /opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.1.0/ARMTools/bin/arm-atollic-eabi-
CROSS_FW_ARM = arm-none-eabi-

CC       =  $(CROSS_FW_ARM)gcc
LD       =  $(CROSS_FW_ARM)gcc
AS       =  $(CROSS_FW_ARM)as
OBJCOPY  =  $(CROSS_FW_ARM)objcopy
OBJDUMP  =  $(CROSS_FW_ARM)objdump
STRIP    =  $(CROSS_FW_ARM)strip
SIZE     =  $(CROSS_FW_ARM)size
GDB      =  $(CROSS_FW_ARM)gdb

FWCRC	 = $(EMBEDX_ROOT)/tools/fwcrc/fwcrc.linux
#OPENOCD = /opt/openocd-0.10/bin/openocd
OPENOCD = openocd

FLASH_FILE = $(PROJECT_TARGET).bin
FLASH_FILE_BOOT = $(BUILD_DIR)/boot.$(PROJECT).$(TARGET).bin
FLASH_ADDR = 0x8000000

CFLAGS += -mthumb
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mlittle-endian 
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -fno-strict-aliasing
CFLAGS += -DUSE_STDPERIPH_DRIVER

LDFLAGS += -mthumb
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -ffunction-sections
LDFLAGS += -fdata-sections
LDFLAGS += -nostartfiles
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(PROJECT_TARGET).map -lm


## CPU depends
ifeq ($(CPU),STM32F469)

 ARM = ARMCM4_FP
 CFLAGS += -DSTM32F469xx
 PROJECT_SOURCEFILES += startup_stm32f469xx.S
 PROJECT_SOURCEFILES += system_stm32f469.c

 ifeq ($(LINKMODE),boot)
  CFLAGS  += -DVECT_TAB_OFFSET=$(CFG_BOOTLOADER_SIZE)
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f469_flash_boot.ld
 else
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f469_flash.ld
 endif

else ifeq ($(CPU),STM32F429)

 ARM = ARMCM4_FP
 CFLAGS += -DSTM32F429xx
 PROJECT_SOURCEFILES += startup_stm32f429xx.S
 PROJECT_SOURCEFILES += system_stm32f429.c

 ifeq ($(LINKMODE),boot)
  CFLAGS  += -DVECT_TAB_OFFSET=$(CFG_BOOTLOADER_SIZE)
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f429_flash_boot.ld
 else
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f429_flash.ld
 endif

else
 
 ARM = ARMCM4_FP
 CFLAGS += -DSTM32F407xx
 PROJECT_SOURCEFILES += startup_stm32f407xx.S
 PROJECT_SOURCEFILES += system_stm32f427.c

 ifeq ($(LINKMODE),boot)
  CFLAGS  += -DVECT_TAB_OFFSET=$(CFG_BOOTLOADER_SIZE)
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f4xx_flash_boot.ld
 else
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f4xx_flash.ld
 endif

endif


## ARM depends
ifeq ($(ARM),ARMCM4_FP)

	CFLAGS += -DARM_MATH_CM4 
	CFLAGS += -DARMCM4_FP 
	CFLAGS += -D__FPU_PRESENT
	CFLAGS += -mfloat-abi=hard 
	CFLAGS += -mfpu=fpv4-sp-d16

	LDFLAGS += -mfloat-abi=hard 
	LDFLAGS += -mfpu=fpv4-sp-d16

	# CMSIS ARM functions
	MODULES += cpu/stm32f4/cmsis/src/TransformFunctions
	MODULES += cpu/stm32f4/cmsis/src/CommonTables
	MODULES += cpu/stm32f4/cmsis/src/ComplexMathFunctions
	MODULES += cpu/stm32f4/cmsis/src/StatisticsFunctions

	#PROJECT_CPU_DIRS    += cmsis/src/TransformFunctions cmsis/src/ComplexMathFunctions cmsis/src/StatisticsFunctions cmsis/src/CommonTables cmsis/src/BasicMathFunctions
	#PROJECT_SOURCEFILES += TransformFunctions.c
	#PROJECT_SOURCEFILES += ComplexMathFunctions.c
	#PROJECT_SOURCEFILES += StatisticsFunctions.c
	#PROJECT_SOURCEFILES += CommonTables.c
	#PROJECT_SOURCEFILES += BasicMathFunctions.c

endif


### Rules
makebin:
	$(Q)$(OBJCOPY) -O binary $(PROJECT_TARGET) $(PROJECT_TARGET).bin
#	$(Q)$(FWCRC) $(PROJECT_TARGET).bin $(CFG_APP_HEADER_OFFSET)
	$(Q)$(SIZE) -A $(PROJECT_TARGET)
	$(Q)$(SIZE) $(PROJECT_TARGET)

load:
	echo $(FLASH_FILE)
	$(OPENOCD) -c "set FLASH_FILE $(FLASH_FILE)" -c "set FLASH_ADDR $(FLASH_ADDR)" -s $(PROJECT_CPU)/openocd -f stm32f4x_prog.cfg

load_boot:
	$(OPENOCD) -c "set FLASH_FILE $(FLASH_FILE_BOOT)" -c "set FLASH_ADDR $(FLASH_ADDR)" -s $(PROJECT_CPU)/openocd -f stm32f4x_prog.cfg

openocd:
	$(OPENOCD) -s $(PROJECT_CPU)/openocd -f stm32f4x_debug.cfg

debug:
	$(GDB) --eval-command="target remote localhost:3333" $(PROJECT_TARGET)

reset:
	$(OPENOCD) -s $(PROJECT_CPU)/openocd -f stm32f4x_reset.cfg

