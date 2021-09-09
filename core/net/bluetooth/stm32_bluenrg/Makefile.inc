
CFLAGS += -DBLUENRG2_DEVICE 
CFLAGS += -DDEBUG -DHCI_LOG_ON

MODULES += core/net/bluetooth/stm32_bluenrg
MODULES += core/net/bluetooth/stm32_bluenrg/hal/inc
MODULES += core/net/bluetooth/stm32_bluenrg/hal/src
MODULES += core/net/bluetooth/stm32_bluenrg/simple_bluenrg_hci/includes
MODULES += core/net/bluetooth/stm32_bluenrg/simple_bluenrg_hci/hci
MODULES += core/net/bluetooth/stm32_bluenrg/simple_bluenrg_hci/hci/controller
