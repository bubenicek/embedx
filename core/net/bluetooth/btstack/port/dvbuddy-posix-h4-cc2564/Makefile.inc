
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/btstack/3rd-party/rijndael
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/btstack/3rd-party/micro-ecc

PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/btstack/src
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/btstack/src/ble

PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/btstack/port/dvbuddy-posix-h4
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/btstack/chipset/cc256x
PROJECT_DIRS += $(EMBEDX_ROOT)/core/net/btstack/platform/posix

PROJECT_SOURCEFILES += bluetooth_init_cc2564B_1.8_BT_Spec_4.1.c
PROJECT_SOURCEFILES += btstack_chipset_cc256x.c
PROJECT_SOURCEFILES += btstack_run_loop_posix.c
PROJECT_SOURCEFILES += btstack_tlv_posix.c
PROJECT_SOURCEFILES += btstack_uart_posix.c
PROJECT_SOURCEFILES += hci_dump_posix_fs.c
PROJECT_SOURCEFILES += hci_transport_h4.c
PROJECT_SOURCEFILES += le_device_db_tlv.c
PROJECT_SOURCEFILES += rijndael.c
PROJECT_SOURCEFILES += btstack_stdin_posix.c
PROJECT_SOURCEFILES += ble_dvbuddy_posix_h4.c

#PROJECT_SOURCEFILES += btstack_link_key_db_tlv.c
#PROJECT_SOURCEFILES += wav_util.c
#PROJECT_SOURCEFILES += 0000000_META_hci_patches_v7.c
#PROJECT_SOURCEFILES += btstack_audio.c
#PROJECT_SOURCEFILES += btstack_audio_portaudio.c

## Core
PROJECT_SOURCEFILES += btstack_memory.c
PROJECT_SOURCEFILES += btstack_linked_list.c
PROJECT_SOURCEFILES += btstack_memory_pool.c
PROJECT_SOURCEFILES += btstack_run_loop.c
PROJECT_SOURCEFILES += btstack_util.c

## Common
PROJECT_SOURCEFILES += ad_parser.c
PROJECT_SOURCEFILES += hci.c
PROJECT_SOURCEFILES += hci_cmd.c
PROJECT_SOURCEFILES += hci_dump.c
PROJECT_SOURCEFILES += l2cap.c
PROJECT_SOURCEFILES += l2cap_signaling.c
PROJECT_SOURCEFILES += btstack_audio.c
PROJECT_SOURCEFILES += btstack_tlv.c
PROJECT_SOURCEFILES += btstack_crypto.c
PROJECT_SOURCEFILES += uECC.c
PROJECT_SOURCEFILES += sm.c
PROJECT_SOURCEFILES += gatt_client.c
PROJECT_SOURCEFILES += att_server.c
PROJECT_SOURCEFILES += att_dispatch.c
PROJECT_SOURCEFILES += att_db.c
