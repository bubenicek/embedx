deps_config := \
	/opt/esp32/esp-idf/components/app_trace/Kconfig \
	/opt/esp32/esp-idf/components/aws_iot/Kconfig \
	/opt/esp32/esp-idf/components/bt/Kconfig \
	/opt/esp32/esp-idf/components/driver/Kconfig \
	/opt/esp32/esp-idf/components/esp32/Kconfig \
	/opt/esp32/esp-idf/components/esp_adc_cal/Kconfig \
	/opt/esp32/esp-idf/components/esp_http_client/Kconfig \
	/opt/esp32/esp-idf/components/ethernet/Kconfig \
	/opt/esp32/esp-idf/components/fatfs/Kconfig \
	/opt/esp32/esp-idf/components/freertos/Kconfig \
	/opt/esp32/esp-idf/components/heap/Kconfig \
	/opt/esp32/esp-idf/components/http_server/Kconfig \
	/opt/esp32/esp-idf/components/libsodium/Kconfig \
	/opt/esp32/esp-idf/components/log/Kconfig \
	/opt/esp32/esp-idf/components/lwip/Kconfig \
	/opt/esp32/esp-idf/components/mbedtls/Kconfig \
	/opt/esp32/esp-idf/components/mdns/Kconfig \
	/opt/esp32/esp-idf/components/mqtt/Kconfig \
	/opt/esp32/esp-idf/components/nvs_flash/Kconfig \
	/opt/esp32/esp-idf/components/openssl/Kconfig \
	/opt/esp32/esp-idf/components/pthread/Kconfig \
	/opt/esp32/esp-idf/components/spi_flash/Kconfig \
	/opt/esp32/esp-idf/components/spiffs/Kconfig \
	/opt/esp32/esp-idf/components/tcpip_adapter/Kconfig \
	/opt/esp32/esp-idf/components/vfs/Kconfig \
	/opt/esp32/esp-idf/components/wear_levelling/Kconfig \
	/opt/esp32/esp-idf/components/bootloader/Kconfig.projbuild \
	/opt/esp32/esp-idf/components/esptool_py/Kconfig.projbuild \
	/opt/esp32/esp-idf/components/partition_table/Kconfig.projbuild \
	/opt/esp32/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
