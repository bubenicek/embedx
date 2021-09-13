
PROJECT_CPU = $(EMBEDX_ROOT)/cpu/stm32f1

PROJECT_CPU_DIRS = . hal cmsis/include drivers/src drivers/include
PROJECT_CPU_DIRS += drivers/src/usbdev drivers/include/usbdev
PROJECT_CPU_DIRS += drivers/src/usbdev/core drivers/include/usbdev/core
#PROJECT_CPU_DIRS += drivers/src/usbdev/class/rndis

PROJECT_SOURCEFILES += system_stm32f1xx.c
PROJECT_SOURCEFILES += stm32f1xx_hal.c
PROJECT_SOURCEFILES += stm32f1xx_hal_cortex.c
PROJECT_SOURCEFILES += stm32f1xx_hal_dma.c
PROJECT_SOURCEFILES += stm32f1xx_hal_flash.c
PROJECT_SOURCEFILES += stm32f1xx_hal_flash_ex.c
PROJECT_SOURCEFILES += stm32f1xx_hal_gpio.c
PROJECT_SOURCEFILES += stm32f1xx_hal_pcd.c
PROJECT_SOURCEFILES += stm32f1xx_hal_pcd_ex.c
PROJECT_SOURCEFILES += stm32f1xx_hal_pwr.c
PROJECT_SOURCEFILES += stm32f1xx_hal_rcc.c
PROJECT_SOURCEFILES += stm32f1xx_hal_rcc_ex.c
PROJECT_SOURCEFILES += stm32f1xx_hal_tim.c
PROJECT_SOURCEFILES += stm32f1xx_hal_tim_ex.c
PROJECT_SOURCEFILES += stm32f1xx_hal_spi.c
PROJECT_SOURCEFILES += stm32f1xx_hal_uart.c
PROJECT_SOURCEFILES += stm32f1xx_hal_adc.c
PROJECT_SOURCEFILES += stm32f1xx_hal_can.c
PROJECT_SOURCEFILES += stm32f1xx_hal_wwdg.c
PROJECT_SOURCEFILES += stm32f1xx_ll_usb.c

# HAL
PROJECT_SOURCEFILES += hal_console.c
PROJECT_SOURCEFILES += hal_time.c
PROJECT_SOURCEFILES += hal_gpio.c
PROJECT_SOURCEFILES += hal_spi.c
PROJECT_SOURCEFILES += hal_uart.c
PROJECT_SOURCEFILES += hal_can.c
PROJECT_SOURCEFILES += hal_wdg.c
PROJECT_SOURCEFILES += hal_pm.c
PROJECT_SOURCEFILES += hal_adc.c
PROJECT_SOURCEFILES += hal_uuid.c
PROJECT_SOURCEFILES += hal_usbserial.c
PROJECT_SOURCEFILES += hal_usbmsc.c
PROJECT_SOURCEFILES += swo.c
PROJECT_SOURCEFILES += dwt_delay.c
PROJECT_SOURCEFILES += hal_flash.c
PROJECT_SOURCEFILES += hal_timer.c

ifeq ($(CPU_USB_DEVICE),MSC)
 ## USB MSC storage
 PROJECT_SOURCEFILES += usbd_core.c usbd_ctlreq.c usbd_ioreq.c
 PROJECT_CPU_DIRS += drivers/src/usbdev/class/msc drivers/include/usbdev/class/msc
 PROJECT_SOURCEFILES += usbd_msc.c
 PROJECT_SOURCEFILES += usbd_msc_bot.c
 PROJECT_SOURCEFILES += usbd_msc_data.c
 PROJECT_SOURCEFILES += usbd_msc_scsi.c
 PROJECT_SOURCEFILES += usb_device.c
 PROJECT_SOURCEFILES += usbd_desc.c
 PROJECT_SOURCEFILES += usbd_storage_if.c
 PROJECT_SOURCEFILES += usbd_conf.c
 CFLAGS += -DCPU_USB_DEVICE_MSC
else ifeq ($(CPU_USB_DEVICE),CDC)
 ## USB CDC
 PROJECT_SOURCEFILES += usbd_core.c usbd_ctlreq.c usbd_ioreq.c
 PROJECT_CPU_DIRS += drivers/src/usbdev/class/cdc drivers/include/usbdev/class/cdc
 PROJECT_SOURCEFILES += usbd_conf.c
 PROJECT_SOURCEFILES += usbd_desc.c
 PROJECT_SOURCEFILES += usb_device.c
 PROJECT_SOURCEFILES += usbd_cdc.c
 PROJECT_SOURCEFILES += usbd_cdc_if.c
 CFLAGS += -DCPU_USB_DEVICE_CDC
endif

## USB RNDIS
#PROJECT_SOURCEFILES += usbd_desc.c


### Compiler definitions
CC       =  $(CROSS_FW_ARM)gcc
LD       =  $(CROSS_FW_ARM)gcc
AS       =  $(CROSS_FW_ARM)as
OBJCOPY  =  $(CROSS_FW_ARM)objcopy
OBJDUMP  =  $(CROSS_FW_ARM)objdump
STRIP    =  $(CROSS_FW_ARM)strip
SIZE     =  $(CROSS_FW_ARM)size
GDB      =  $(CROSS_FW_ARM)gdb
FWCRC	 	= $(ROOT)/tools/fwcrc/fwcrc


FLASH_FILE = $(PROJECT_TARGET).bin
FLASH_FILE_BOOT = boot.$(PROJECT_TARGET).bin
FLASH_ADDR = 0x8000000

CFLAGS += -mlittle-endian -mthumb
CFLAGS += -mcpu=cortex-m3
CFLAGS += -mfloat-abi=soft
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -fno-strict-aliasing

LDFLAGS += -mthumb-interwork
LDFLAGS += -mlittle-endian -mthumb
LDFLAGS += -mcpu=cortex-m3
LDFLAGS += -mfloat-abi=soft
LDFLAGS += -ffunction-sections
LDFLAGS += -fdata-sections
LDFLAGS += -specs=nosys.specs -u _printf_float -static -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x80 -Wl,--start-group -lc -lm -Wl,--end-group -specs=nano.specs
LDFLAGS += -Wl,-Map=$(PROJECT_TARGET).map -lm


## CPU depends
ifeq ($(CPU),STM32F103xB)
 ##
 ## STM32F103RB6 64KB RAM, 128B FLASH
 ##
 PROJECT_SOURCEFILES += startup_stm32f103xb.S
 CFLAGS += -DSTM32F103xB

 ifeq ($(LINKMODE),boot)
  CFLAGS += -DVECT_TAB_OFFSET=$(CFG_BOOTLOADER_SIZE)
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f103c8_flash_boot.ld
 else
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f103c8_flash.ld
 endif

else ifeq ($(CPU),STM32F103xE)
 ##
 ## STM32F103VE 64KB RAM, 512KB FLASH
 ##
 PROJECT_SOURCEFILES += startup_stm32f103xe.S
 CFLAGS += -DSTM32F103xE

 ifeq ($(LINKMODE),boot)
  CFLAGS += -DVECT_TAB_OFFSET=$(CFG_BOOTLOADER_SIZE)
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f103ve_flash_boot.ld
 else
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f103ve_flash.ld
 endif

else ifeq ($(CPU),STM32F107xC)
 ##
 ## STM32F103VE 64KB RAM, 512KB FLASH
 ##
 PROJECT_SOURCEFILES += startup_stm32f107xc.S
 PROJECT_SOURCEFILES += stm32f1xx_hal_eth.c
 PROJECT_SOURCEFILES += hal_net.c
 
 CFLAGS += -DSTM32F107xC

 ifeq ($(LINKMODE),boot)
  CFLAGS += -DVECT_TAB_OFFSET=$(CFG_BOOTLOADER_SIZE)
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f107rb_flash_boot.ld
 else
  LDFLAGS += -T$(PROJECT_CPU)/linker_scripts/stm32f107rb_flash.ld
 endif

else
   abort
endif


## Rules
makebin:
	$(Q)$(OBJCOPY) -O binary $(PROJECT_TARGET) $(PROJECT_TARGET).bin
#	$(Q)$(FWCRC) $(PROJECT_TARGET).bin $(CFG_APP_HEADER_OFFSET)
	$(Q)$(SIZE) -A $(PROJECT_TARGET)
	$(Q)$(SIZE) $(PROJECT_TARGET)

makebin_boot:
	$(Q)cp ../bootloader/bootloader.$(TARGET).bin boot.$(PROJECT).$(TARGET).bin
	$(Q)cat $(PROJECT).$(TARGET).bin >> boot.$(PROJECT).$(TARGET).bin

load:
	$(OPENOCD) -c "set FLASH_FILE $(FLASH_FILE)" -c "set FLASH_ADDR $(FLASH_ADDR)" -s $(PROJECT_CPU)/openocd -f stm32f1x_prog.cfg

load_boot:
	$(OPENOCD) -c "set FLASH_FILE $(FLASH_FILE_BOOT)" -c "set FLASH_ADDR $(FLASH_ADDR)" -s $(PROJECT_CPU)/openocd -f stm32f1x_prog.cfg

openocd:
	$(OPENOCD) -s $(PROJECT_CPU)/openocd -f stm32f1x_debug.cfg

debug:
	$(GDB) --eval-command="target remote localhost:3333" $(PROJECT_TARGET)

reset:
	$(OPENOCD) -s $(PROJECT_CPU)/openocd -f stm32f1x_reset.cfg
