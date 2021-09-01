#define __BTSTACK_FILE__ "main.c"

// *****************************************************************************
//
// minimal setup for HCI code
//
// *****************************************************************************

#include "system.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "btstack_config.h"

#include "btstack_audio.h"
#include "btstack_debug.h"
#include "btstack_event.h"
#include "ble/le_device_db_tlv.h"
#include "classic/btstack_link_key_db_tlv.h"
#include "btstack_memory.h"
#include "btstack_run_loop.h"
#include "btstack_run_loop_posix.h"
#include "btstack_uart.h"
#include "bluetooth_company_id.h"
#include "hci.h"
#include "hci_dump.h"
#include "hci_dump_posix_fs.h"
#include "hci_transport.h"
#include "hci_transport_h4.h"
#include "btstack_stdin.h"
#include "btstack_tlv_posix.h"

#include "btstack_chipset_cc256x.h"

TRACE_TAG(ble_dvb);


#define TLV_DB_PATH_PREFIX "/tmp/btstack_"
#define TLV_DB_PATH_POSTFIX ".tlv"
static char tlv_db_path[100];
static const btstack_tlv_t *tlv_impl;
static btstack_tlv_posix_t tlv_context;

int btstack_main(int argc, const char *argv[]);
static void local_version_information_handler(uint8_t *packet);

static hci_transport_config_uart_t config = {
    HCI_TRANSPORT_CONFIG_UART,
    115200,
    115200, // main baudrate
    0,      // flow control
    NULL,
};

static btstack_packet_callback_registration_t hci_event_callback_registration;

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    bd_addr_t addr;

    //TRACE("Packet handler  type: %d  channel: %d", packet_type, channel);
    //TRACE_DUMP(packet, size);

    if (packet_type != HCI_EVENT_PACKET)
        return;
    
    switch (hci_event_packet_get_type(packet))
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
            break;
        gap_local_bd_addr(addr);
        printf("BTstack up and running at %s\n", bd_addr_to_str(addr));

        // setup TLV
        strcpy(tlv_db_path, TLV_DB_PATH_PREFIX);
        strcat(tlv_db_path, bd_addr_to_str(addr));
        strcat(tlv_db_path, TLV_DB_PATH_POSTFIX);
        tlv_impl = btstack_tlv_posix_init_instance(&tlv_context, tlv_db_path);
        btstack_tlv_set_instance(tlv_impl, &tlv_context);

        le_device_db_tlv_configure(tlv_impl, &tlv_context);
        break;

    case HCI_EVENT_COMMAND_COMPLETE:
        if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_read_local_name))
        {
            if (hci_event_command_complete_get_return_parameters(packet)[0])
                break;
            // terminate, name 248 chars
            packet[6 + 248] = 0;
            printf("Local name: %s\n", &packet[6]);
        }
        if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_read_local_version_information))
        {
            local_version_information_handler(packet);
        }
        break;

    default:
        TRACE_ERROR("Not supported HCI event 0x%X\n", hci_event_packet_get_type(packet));
        break;
    }
}

static void sigint_handler(int param)
{
    printf("CTRL-C - SIGINT received, shutting down..\n");

    // reset anyway
    btstack_stdin_reset();

    // power down
    hci_power_control(HCI_POWER_OFF);
    hci_close();

    exit(0);
}

static void local_version_information_handler(uint8_t *packet)
{
    printf("Local version information:\n");
    uint16_t hci_version = packet[6];
    uint16_t hci_revision = little_endian_read_16(packet, 7);
    uint16_t lmp_version = packet[9];
    uint16_t manufacturer = little_endian_read_16(packet, 10);
    uint16_t lmp_subversion = little_endian_read_16(packet, 12);
    printf("- HCI Version    0x%04x\n", hci_version);
    printf("- HCI Revision   0x%04x\n", hci_revision);
    printf("- LMP Version    0x%04x\n", lmp_version);
    printf("- LMP Subversion 0x%04x\n", lmp_subversion);
    printf("- Manufacturer 0x%04x\n", manufacturer);
    switch (manufacturer)
    {
/*        
    case BLUETOOTH_COMPANY_ID_TEXAS_INSTRUMENTS_INC:
        printf("Texas Instruments - CC256x compatible chipset.\n");
        if (lmp_subversion != btstack_chipset_cc256x_lmp_subversion())
        {
            printf("Error: LMP Subversion does not match initscript! ");
            printf("Your initscripts is for %s chipset\n", btstack_chipset_cc256x_lmp_subversion() < lmp_subversion ? "an older" : "a newer");
            printf("Please update Makefile to include the appropriate bluetooth_init_cc256???.c file\n");
            exit(10);
        }
        hci_set_chipset(btstack_chipset_cc256x_instance());
#ifdef ENABLE_EHCILL
        printf("eHCILL enabled.\n");
#else
        printf("eHCILL disable.\n");
#endif

        break;
*/        
    default:
        printf("Unknown manufacturer / manufacturer not supported yet.\n");
        break;
    }
}

int ble_dvbuddy_btstack_init(void)
{
    /// GET STARTED with BTstack ///
    btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_posix_get_instance());

    config.device_name = "/dev/ttyUSB0";
    printf("H4 device: %s\n", config.device_name);

   // log into file using HCI_DUMP_PACKETLOGGER format
    const char * pklg_path = "/tmp/hci_dump.pklg";
    hci_dump_posix_fs_open(pklg_path, HCI_DUMP_PACKETLOGGER);
    const hci_dump_t * hci_dump_impl = hci_dump_posix_fs_get_instance();
    hci_dump_init(hci_dump_impl);
    printf("Packet Log: %s\n", pklg_path);

    // init HCI
    const btstack_uart_t *uart_driver = btstack_uart_posix_instance();
    const hci_transport_t *transport = hci_transport_h4_instance_for_uart(uart_driver);
    hci_init(transport, (void *)&config);

    // set BD_ADDR for CSR without Flash/unique address
    // bd_addr_t own_address = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    // btstack_chipset_csr_set_bd_addr(own_address);

    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // handle CTRL-c
    signal(SIGINT, sigint_handler);

    return 0;
}

int ble_dvbuddy_btstack_run(void)
{
    // go
    btstack_run_loop_execute();

    return 0;
}
