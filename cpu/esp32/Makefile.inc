
PROJECT_CPU = $(EMBEDX_ROOT)/cpu/esp32
PROJECT_CPU_DIRS = . hal

# HAL
PROJECT_SOURCEFILES += hal_console.c
PROJECT_SOURCEFILES += hal_time.c
PROJECT_SOURCEFILES += hal_gpio.c
PROJECT_SOURCEFILES += hal_dmx.c
PROJECT_SOURCEFILES += hal_uart.c
PROJECT_SOURCEFILES += hal_pm.c
PROJECT_SOURCEFILES += hal_net.c
PROJECT_SOURCEFILES += hal_i2c.c
PROJECT_SOURCEFILES += hal_rtc.c
PROJECT_SOURCEFILES += hal_xmem.c
PROJECT_SOURCEFILES += twi.c
PROJECT_SOURCEFILES += xclk.c
PROJECT_SOURCEFILES += wiring.c

############################
### Compiler definitions ###
############################
CROSSCOMPILER_DIR = /opt/esp32/xtensa-esp32-elf

CC       = $(CROSSCOMPILER_DIR)/bin/xtensa-esp32-elf-gcc
LD       = $(CROSSCOMPILER_DIR)/bin/xtensa-esp32-elf-gcc
AS       = $(CROSSCOMPILER_DIR)/bin/xtensa-esp32-elf-as
OBJCOPY  = $(CROSSCOMPILER_DIR)/bin/xtensa-esp32-elf-objcopy
STRIP    = $(CROSSCOMPILER_DIR)/bin/xtensa-esp32-elf-strip
OBJDUMP	 = $(CROSSCOMPILER_DIR)/bin/xtensa-esp32-elf-objdump
SIZE     = $(CROSSCOMPILER_DIR)/bin/xtensa-esp32-elf-size
#ESPTOOL = $(PROJECT_CPU)/components/esptool_py/esptool/esptool.py
ESPTOOL	 = esptool.py
ESPSIZE  = $(PROJECT_CPU)/tools/idf_size.py

PARTITION_FILE = $(PROJECT_CPU)/build/partitions_ota.bin

CFLAGS += -std=gnu99 -Og -ggdb -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib
CFLAGS += -DESP_PLATFORM -D IDF_VER=\"v3.2-dev-1055-g3276a13\" -MMD -MP   -DWITH_POSIX -DMBEDTLS_CONFIG_FILE='"mbedtls/esp_config.h"' -DHAVE_CONFIG_H

CFLAGS += -I $(PROJECT_CPU)/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/ar/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/av/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/hf_client/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/dm/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/gatt/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/hh/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/jv/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/sdp/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/bta/sys/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btcore/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/device/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/gki/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/hci/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/osi/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/utils/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/external/sbc/decoder/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/external/sbc/encoder/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btc/core/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btc/profile/esp/blufi/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btc/profile/esp/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btc/profile/std/gatt/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btc/profile/std/gap/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btc/profile/std/a2dp/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btc/profile/std/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btc/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/btif/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/btm/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/btu/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/gap/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/gatt/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/hcic/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/l2cap/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/sdp/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/smp/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/avct/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/avrc/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/avdt/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/a2dp/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/rfcomm/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/stack/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/utils/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/common/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/include
CFLAGS += -I $(PROJECT_CPU)/components/bt/bluedroid/api/include/api
CFLAGS += -I $(PROJECT_CPU)/components/app_trace/include
CFLAGS += -I $(PROJECT_CPU)/components/app_update/include
CFLAGS += -I $(PROJECT_CPU)/components/asio/asio/asio/include
CFLAGS += -I $(PROJECT_CPU)/components/asio/port/include
CFLAGS += -I $(PROJECT_CPU)/components/bootloader_support/include
CFLAGS += -I $(PROJECT_CPU)/components/btstack/3rd-party/bluedroid/decoder/include
CFLAGS += -I $(PROJECT_CPU)/components/btstack/3rd-party/bluedroid/encoder/include
CFLAGS += -I $(PROJECT_CPU)/components/btstack/3rd-party/hxcmod-player
CFLAGS += -I $(PROJECT_CPU)/components/btstack/3rd-party/hxcmod-player/mods
CFLAGS += -I $(PROJECT_CPU)/components/btstack/src/classic
CFLAGS += -I $(PROJECT_CPU)/components/btstack/src/ble/gatt-service
CFLAGS += -I $(PROJECT_CPU)/components/btstack/src/ble
CFLAGS += -I $(PROJECT_CPU)/components/btstack/src/classic
CFLAGS += -I $(PROJECT_CPU)/components/btstack/src
CFLAGS += -I $(PROJECT_CPU)/components/btstack/platform/embedded
CFLAGS += -I $(PROJECT_CPU)/components/btstack/platform/freertos
CFLAGS += -I $(PROJECT_CPU)/components/btstack/include
CFLAGS += -I $(PROJECT_CPU)/components/coap/port/include
CFLAGS += -I $(PROJECT_CPU)/components/coap/port/include/coap
CFLAGS += -I $(PROJECT_CPU)/components/coap/libcoap/include
CFLAGS += -I $(PROJECT_CPU)/components/coap/libcoap/include/coap
CFLAGS += -I $(PROJECT_CPU)/components/console
CFLAGS += -I $(PROJECT_CPU)/components/driver/include
CFLAGS += -I $(PROJECT_CPU)/components/esp-tls
CFLAGS += -I $(PROJECT_CPU)/components/esp32/include
CFLAGS += -I $(PROJECT_CPU)/components/esp_adc_cal/include
CFLAGS += -I $(PROJECT_CPU)/components/esp_http_client/include
CFLAGS += -I $(PROJECT_CPU)/components/esp_https_ota/include
CFLAGS += -I $(PROJECT_CPU)/components/esp_ringbuf/include
CFLAGS += -I $(PROJECT_CPU)/components/ethernet/include
CFLAGS += -I $(PROJECT_CPU)/components/expat/expat/expat/lib
CFLAGS += -I $(PROJECT_CPU)/components/expat/port/include
CFLAGS += -I $(PROJECT_CPU)/components/fatfs/src
CFLAGS += -I $(PROJECT_CPU)/components/freertos/include
CFLAGS += -I $(PROJECT_CPU)/components/heap/include
CFLAGS += -I $(PROJECT_CPU)/components/http_server/include
CFLAGS += -I $(PROJECT_CPU)/components/idf_test/include
CFLAGS += -I $(PROJECT_CPU)/components/jsmn/include
CFLAGS += -I $(PROJECT_CPU)/components/json/cJSON
CFLAGS += -I $(PROJECT_CPU)/components/libsodium/libsodium/src/libsodium/include
CFLAGS += -I $(PROJECT_CPU)/components/libsodium/port_include
CFLAGS += -I $(PROJECT_CPU)/components/log/include
CFLAGS += -I $(PROJECT_CPU)/components/lwip/include/apps
CFLAGS += -I $(PROJECT_CPU)/components/lwip/lwip/src/include
CFLAGS += -I $(PROJECT_CPU)/components/lwip/port/esp32/include
CFLAGS += -I $(PROJECT_CPU)/components/lwip/port/esp32/include/arch
CFLAGS += -I $(PROJECT_CPU)/components/mbedtls/port/include
CFLAGS += -I $(PROJECT_CPU)/components/mbedtls/mbedtls/include
CFLAGS += -I $(PROJECT_CPU)/components/mdns/include
CFLAGS += -I $(PROJECT_CPU)/components/micro-ecc/micro-ecc
CFLAGS += -I $(PROJECT_CPU)/components/mqtt/esp-mqtt/include
CFLAGS += -I $(PROJECT_CPU)/components/newlib/platform_include
CFLAGS += -I $(PROJECT_CPU)/components/newlib/include
CFLAGS += -I $(PROJECT_CPU)/components/nghttp/port/include
CFLAGS += -I $(PROJECT_CPU)/components/nghttp/nghttp2/lib/includes
CFLAGS += -I $(PROJECT_CPU)/components/nvs_flash/include
CFLAGS += -I $(PROJECT_CPU)/components/openssl/include
CFLAGS += -I $(PROJECT_CPU)/components/pthread/include
CFLAGS += -I $(PROJECT_CPU)/components/sdmmc/include
CFLAGS += -I $(PROJECT_CPU)/components/smartconfig_ack/include
CFLAGS += -I $(PROJECT_CPU)/components/soc/esp32/include
CFLAGS += -I $(PROJECT_CPU)/components/soc/include
CFLAGS += -I $(PROJECT_CPU)/components/spi_flash/include
CFLAGS += -I $(PROJECT_CPU)/components/spiffs/include
CFLAGS += -I $(PROJECT_CPU)/components/tcp_transport/include
CFLAGS += -I $(PROJECT_CPU)/components/tcpip_adapter/include
CFLAGS += -I $(PROJECT_CPU)/components/ulp/include
CFLAGS += -I $(PROJECT_CPU)/components/vfs/include
CFLAGS += -I $(PROJECT_CPU)/components/wear_levelling/include
CFLAGS += -I $(PROJECT_CPU)/components/wpa_supplicant/include
CFLAGS += -I $(PROJECT_CPU)/components/wpa_supplicant/port/include
CFLAGS += -I $(PROJECT_CPU)/components/xtensa-debug-module/include

LDFLAGS +=-Wl,-Map=$(PROJECT_TARGET).map
LIBS += -nostdlib -u call_user_start_cpu0  -Wl,--gc-sections -Wl,-static -Wl,--start-group  -L$(PROJECT_CPU)/esp32-mklibs/build/app_trace -lapp_trace -L$(PROJECT_CPU)/esp32-mklibs/build/app_update -lapp_update -L$(PROJECT_CPU)/esp32-mklibs/build/asio -lasio -L$(PROJECT_CPU)/esp32-mklibs/build/aws_iot  -L$(PROJECT_CPU)/esp32-mklibs/build/bootloader_support -lbootloader_support -L$(PROJECT_CPU)/esp32-mklibs/build/bt -lbt -L /opt/esp32/esp-idf/components/bt/lib -lbtdm_app -L$(PROJECT_CPU)/esp32-mklibs/build/btstack -lbtstack -L$(PROJECT_CPU)/esp32-mklibs/build/coap -lcoap -L$(PROJECT_CPU)/esp32-mklibs/build/console -lconsole -L$(PROJECT_CPU)/esp32-mklibs/build/cxx -lcxx -u __cxa_guard_dummy -u __cxx_fatal_exception -L$(PROJECT_CPU)/esp32-mklibs/build/driver -ldriver -L$(PROJECT_CPU)/esp32-mklibs/build/esp-tls -lesp-tls -L$(PROJECT_CPU)/esp32-mklibs/build/esp32 -lesp32 /opt/esp32/esp-idf/components/esp32/libhal.a -L/opt/esp32/esp-idf/components/esp32/lib -lcore -lrtc -lnet80211 -lpp -lwpa -lsmartconfig -lcoexist -lwps -lwpa2 -lespnow -lphy -lmesh -L /opt/esp32/esp-idf/components/esp32/ld -T esp32_out.ld -u ld_include_panic_highint_hdl -T esp32.common.ld -T esp32.rom.ld -T esp32.peripherals.ld -T esp32.rom.libgcc.ld -T esp32.rom.spiram_incompatible_fns.ld -L$(PROJECT_CPU)/esp32-mklibs/build/esp_adc_cal -lesp_adc_cal -L$(PROJECT_CPU)/esp32-mklibs/build/esp_http_client -lesp_http_client -L$(PROJECT_CPU)/esp32-mklibs/build/esp_https_ota -lesp_https_ota -L$(PROJECT_CPU)/esp32-mklibs/build/esp_ringbuf -lesp_ringbuf -L$(PROJECT_CPU)/esp32-mklibs/build/ethernet -lethernet -L$(PROJECT_CPU)/esp32-mklibs/build/expat -lexpat -L$(PROJECT_CPU)/esp32-mklibs/build/fatfs -lfatfs -L$(PROJECT_CPU)/esp32-mklibs/build/freertos -lfreertos -Wl,--undefined=uxTopUsedPriority -L$(PROJECT_CPU)/esp32-mklibs/build/heap -lheap -L$(PROJECT_CPU)/esp32-mklibs/build/http_server -lhttp_server -L$(PROJECT_CPU)/esp32-mklibs/build/idf_test -lidf_test -L$(PROJECT_CPU)/esp32-mklibs/build/jsmn -ljsmn -L$(PROJECT_CPU)/esp32-mklibs/build/json -ljson -L$(PROJECT_CPU)/esp32-mklibs/build/libsodium -llibsodium -L$(PROJECT_CPU)/esp32-mklibs/build/log -llog -L$(PROJECT_CPU)/esp32-mklibs/build/lwip -llwip -L$(PROJECT_CPU)/esp32-mklibs/build/main -lmain -L$(PROJECT_CPU)/esp32-mklibs/build/mbedtls -lmbedtls -L$(PROJECT_CPU)/esp32-mklibs/build/mdns -lmdns -L$(PROJECT_CPU)/esp32-mklibs/build/micro-ecc -lmicro-ecc -L$(PROJECT_CPU)/esp32-mklibs/build/mqtt -lmqtt -L$(PROJECT_CPU)/esp32-mklibs/build/newlib /opt/esp32/esp-idf/components/newlib/lib/libc.a /opt/esp32/esp-idf/components/newlib/lib/libm.a -lnewlib -L$(PROJECT_CPU)/esp32-mklibs/build/nghttp -lnghttp -L$(PROJECT_CPU)/esp32-mklibs/build/nvs_flash -lnvs_flash -L$(PROJECT_CPU)/esp32-mklibs/build/openssl -lopenssl -L$(PROJECT_CPU)/esp32-mklibs/build/pthread -lpthread -L$(PROJECT_CPU)/esp32-mklibs/build/sdmmc -lsdmmc -L$(PROJECT_CPU)/esp32-mklibs/build/smartconfig_ack -lsmartconfig_ack -L$(PROJECT_CPU)/esp32-mklibs/build/soc -lsoc -L$(PROJECT_CPU)/esp32-mklibs/build/spi_flash -lspi_flash -L$(PROJECT_CPU)/esp32-mklibs/build/spiffs -lspiffs -L$(PROJECT_CPU)/esp32-mklibs/build/tcp_transport -ltcp_transport -L$(PROJECT_CPU)/esp32-mklibs/build/tcpip_adapter -ltcpip_adapter -L$(PROJECT_CPU)/esp32-mklibs/build/ulp -lulp -L$(PROJECT_CPU)/esp32-mklibs/build/vfs -lvfs -L$(PROJECT_CPU)/esp32-mklibs/build/wear_levelling -lwear_levelling -L$(PROJECT_CPU)/esp32-mklibs/build/wpa_supplicant -lwpa_supplicant -L$(PROJECT_CPU)/esp32-mklibs/build/xtensa-debug-module -lxtensa-debug-module -lgcc -lstdc++ -lgcov -Wl,--end-group -Wl,-EL


### Compilation rules
load: erase_flash
	$(ESPTOOL) --chip esp32 --port /dev/ttyUSB0 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 $(PROJECT_CPU)/build/bootloader/bootloader.bin 0x10000 $(PROJECT_TARGET).bin 0x8000 $(PARTITION_FILE)
	terminal0.sh

erase_flash:
	$(ESPTOOL) --chip esp32 --port /dev/ttyUSB0 --baud 921600 --before default_reset --after hard_reset erase_flash  


makebin:
	$(ESPTOOL) --chip esp32 elf2image --flash_mode "dio" --flash_freq "40m" --flash_size "4MB"  -o $(PROJECT_TARGET).bin $(PROJECT_TARGET)
	python $(ESPSIZE) $(PROJECT_TARGET).map
