#include "system.h"

#include "uuid.h"

#include "osal.h"
#include "hci.h"
#include "bluenrg1_hci_le.h"
#include "bluenrg1_gap_aci.h"
#include "bluenrg1_hal_aci.h"
#include "bluenrg1_gatt_aci.h"
#include "bluenrg1_events.h"

TRACE_TAG(bletest);

#define UUID_CHAR_BATTERY       "00002a19-0000-1000-8000-00805f9b34fb"

typedef enum
{
    STATE_IDLE,
    STATE_SCAN_PENDING,
    STATE_CONNECT,
    STATE_CONNECT_PENDING,
    STATE_DISCOVER,
    STATE_DISOVER_PENDING,
    STATE_FIND_CHAR,
    STATE_FIND_CHAR_PENDING,
    STATE_READ_CHAR,
    STATE_READ_CHAR_PENDING,
    STATE_FINISHED

} test_state_t;

typedef struct
{
    uint8_t value[6];

} ble_addr_t;

static void bluenrg_thread(void *arg);
static const osThreadDef(BLUENRG_THREAD, bluenrg_thread, 0, 0, 1024);

static ble_addr_t test_addr;
static test_state_t state = STATE_IDLE;
static uint16_t connection_handle;
static uint16_t attr_handle;

static char char_uuid_battery[16] = {0x00, 0x00, 0x2a, 0x19, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb};


static void bluenrg_thread(void *arg)
{
    TRACE("bluenrg thread started");

    while (1)
    {
        BTLE_StackTick();
        osDelay(20);
    }
}

/**
 * @brief  Init a BlueNRG device
 * @param  None.
 * @retval None.
*/
int ble_device_initialization(void)
{
    uint16_t service_handle;
    uint16_t dev_name_char_handle;
    uint16_t appearance_char_handle;
    uint8_t status;

    status = hci_reset();
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("hci_reset() failed:0x%02x", status);
        return -1;
    }
    TRACE("hci_reset --> SUCCESS");

    // Wait time for reset finished
    osDelay(1000);

    // Get local version
    uint8_t HCI_Version;
    uint16_t HCI_Revision;
    uint8_t LMP_PAL_Version;
    uint16_t Manufacturer_Name;
    uint16_t LMP_PAL_Subversion;

    status = hci_read_local_version_information(&HCI_Version, &HCI_Revision, &LMP_PAL_Version, &Manufacturer_Name, &LMP_PAL_Subversion);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("hci_read_local_version_information failed:0x%02x", status);
        return -1;
    }
    TRACE("HCI_Version: %d  HCI_Revision: %d   LMP_PAL_Version: %d  Manufacturer_Name: %d   LMP_PAL_Subversion: %d --> SUCCESS\n",
          HCI_Version, HCI_Revision, LMP_PAL_Version, Manufacturer_Name, LMP_PAL_Subversion);

    uint8_t value_4[] = {0x58, 0x65, 0x00, 0xE1, 0x80, 0x02};

    //status = aci_hal_write_config_data(offset,length,value);
    status = aci_hal_write_config_data(0x00, 0x06, value_4);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_hal_write_config_data() failed:0x%02x", status);
        return -1;
    }
    TRACE("aci_hal_write_config_data --> SUCCESS");

    //status = aci_hal_set_tx_power_level(en_high_power,pa_level);
    status = aci_hal_set_tx_power_level(0x01, 0x04);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_hal_set_tx_power_level() failed:0x%02x", status);
        return -1;
    }
    TRACE("aci_hal_set_tx_power_level --> SUCCESS");

    status = aci_gatt_init();
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gatt_init() failed:0x%02x", status);
        return -1;
    }
    TRACE("aci_gatt_init --> SUCCESS");

    //status = aci_gap_init(role,privacy_enabled,device_name_char_len, &service_handle, &dev_name_char_handle, &appearance_char_handle);
    status = aci_gap_init(GAP_CENTRAL_ROLE, 0x00, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gap_init() failed:0x%02x", status);
        return -1;
    }
    TRACE("aci_gap_init --> SUCCESS");

    uint8_t char_value_12[] = {0x42, 0x6C, 0x75, 0x65, 0x4E, 0x52, 0x47};

    //status = aci_gatt_update_char_value(service_handle,char_handle,val_offset,char_value_length,char_value);
    status = aci_gatt_update_char_value(0x0005, 0x0006, 0x00, 0x07, char_value_12);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gatt_update_char_value() failed:0x%02x", status);
        return -1;
    }
    TRACE("aci_gatt_update_char_value --> SUCCESS");

/*
    // Update characteristics
    uint8_t char_value_16[] = {0x42,0x6C,0x75,0x65,0x4E,0x52,0x47};

    status = aci_gatt_update_char_value(0x0005,0x0006,0x00,0x07,char_value_16);
    if (status != BLE_STATUS_SUCCESS) 
    {
        TRACE_ERROR("aci_gatt_update_char_value() failed:0x%02x", status);
        return -1;
    }
    TRACE("aci_gatt_update_char_value --> SUCCESS");
*/
    return 0;
}

int ble_start_scan(void)
{
    uint8_t status;

    status = aci_gap_start_general_discovery_proc(0x4000, 0x4000, 0x00, 0x00);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("Start BLE scan failed, status: 0x%X", status);
        return -1;
    }
    TRACE("aci_gap_start_general_discovery_proc --> SUCCESS");

    return 0;
}

int ble_stop_scan(void)
{
    uint8_t status;

    status = aci_gap_terminate_gap_proc(0x02);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("Stop BLE scan failed, status: 0x%X", status);
        return -1;
    }
    TRACE("aci_gap_terminate_gap_proc --> SUCCESS");

    return 0;
}

int ble_connect(ble_addr_t *addr)
{
    uint8_t status;

    //status = aci_gap_create_connection(le_scan_interval,le_scan_window,peer_address_type,peer_address,own_address_type,conn_interval_min,conn_interval_max,conn_latency,supervision_timeout,minimum_ce_length,maximum_ce_length);
    status = aci_gap_create_connection(0x4000, 0x4000, 0x01, addr->value, 0x00, 0x0006, 0x0028, 0x0000, 0x03E8, 0x0000, 0x03E8);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gap_create_connection() failed:0x%02x\r\n", status);
        return -1;
    }

    TRACE("aci_gap_create_connection --> SUCCESS");

    return 0;
}

int ble_discovery_services(void)
{
    uint8_t status;

    status = aci_gatt_disc_all_primary_services(0x0801);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gatt_disc_all_primary_services() failed:0x%02x", status);
    }
    TRACE("aci_gatt_disc_all_primary_services --> SUCCESS");

    status = aci_gatt_disc_all_char_of_service(0x0801, 0x0001, 0x0003);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gatt_disc_all_char_of_service() failed:0x%02x", status);
    }
    TRACE("aci_gatt_disc_all_char_of_service --> SUCCESS");

    status = aci_gatt_disc_all_char_of_service(0x0801, 0x0014, 0x001A);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gatt_disc_all_char_of_service() failed:0x%02x", status);
    }
    TRACE("aci_gatt_disc_all_char_of_service --> SUCCESSG");

    status = aci_gatt_disc_all_char_of_service(0x0801, 0x0028, 0xFFFF);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gatt_disc_all_char_of_service() failed:0x%02x", status);
    }
    TRACE("aci_gatt_disc_all_char_of_service --> SUCCESS");

    status = aci_gatt_disc_all_char_desc(0x0801, 0x002A, 0xFFFF);
    if (status != BLE_STATUS_SUCCESS)
    {
        TRACE_ERROR("aci_gatt_disc_all_char_desc() failed:0x%02x", status);
    }
    TRACE("aci_gatt_disc_all_char_desc --> SUCCESS");

    return 0;
}

int bletest(void)
{
    int c;
    tBleStatus status;

    // Initialize UART
    ASSERT(hal_uart_init(BLUENRG_UART) == 0);

    // Initialze BLE stack
    ASSERT(BlueNRG_Stack_Initialization() == 0);
    TRACE_PRINTF("\n");

    // Start receive events thread
    ASSERT(osThreadCreate(osThread(BLUENRG_THREAD), NULL) != 0);

    // Initialize BLE device
    ASSERT(ble_device_initialization() == 0);
    TRACE_PRINTF("\n");

    while(1)
    {
        switch(state)
        {
            case STATE_IDLE:
            {
                TRACE_PRINTF("\n");

                // Scan devices
                ASSERT(ble_start_scan() == 0);
                state = STATE_SCAN_PENDING;
            }
            break;

            case STATE_CONNECT:
            {
                TRACE_PRINTF("\n");

                // Connect to founded device
                TRACE("Found device, connecting ...");
                ASSERT(ble_connect(&test_addr) == 0);

                state = STATE_CONNECT_PENDING;
            }
            break;

            case STATE_DISCOVER:
            {
                TRACE_PRINTF("\n");

                // Discovery services
                ASSERT(ble_discovery_services() == 0);
                TRACE_PRINTF("\n");
                state = STATE_DISOVER_PENDING;
            }
            break;

            case STATE_FIND_CHAR:
            {
                UUID_t uuid;
                uint8_t temp[16];

                TRACE_PRINTF("\n");

                uuid128_parse(UUID_CHAR_BATTERY, temp, 16);

                // Revert bytes order
                for (int i = 0; i < 16; i++) {
                    uuid.UUID_128[15-i] = temp[i];
                }

                int status = aci_gatt_disc_char_by_uuid(connection_handle, 0x0001, 0xFFFF, UUID_TYPE_128, &uuid);
                if (status != BLE_STATUS_SUCCESS) {
                    TRACE_ERROR("aci_gatt_disc_char_by_uuid failed: 0x%x", status);
                    state = STATE_FINISHED;
                }
                else {
                    TRACE("aci_gatt_disc_char_by_uuid --> SUCCESS");
                    state = STATE_FIND_CHAR_PENDING;
                }               
            }
            break;

            case STATE_READ_CHAR:
            {
                TRACE_PRINTF("\n");

                int status = aci_gatt_read_char_value(connection_handle, attr_handle);
                if (status != BLE_STATUS_SUCCESS) {
                    TRACE_ERROR("aci_gatt_read_char_value failed: 0x%x", status);
                    state = STATE_FINISHED;
                }
                else {
                    TRACE("aci_gatt_read_char_value attr_handle: 0x%x--> SUCCESS", attr_handle);
                    state = STATE_READ_CHAR_PENDING;
                }
            }
            break;

            case STATE_FINISHED:
            {
                TRACE("**** Test finished ***");
                exit(0);
            }
        }

        osDelay(1000);
    }

    return 0;
}

//
// ACI HAL events
//

/**
  * @brief This event inform the application that the network coprocessor has been reset. If the reason code is a system crash,
a following event @ref aci_blue_crash_info_event will provide more information regarding the system crash details.
  * @param Reason_Code Reason code describing why device was reset and in which mode is operating (Updater or Normal mode)
  * Values:
  - 0x01: Firmware started properly
  - 0x02: Updater mode entered with ACI command
  - 0x03: Updater mode entered due to bad Blue Flag
  - 0x04: Updater mode entered due to IRQ pin
  - 0x05: System reset due to watchdog
  - 0x06: System reset due to lockup
  - 0x07: System reset due to brownout reset
  - 0x08: System reset due to crash
  - 0x09: System reset due to ECC error
  * @retval None
*/
void aci_blue_initialized_event(uint8_t Reason_Code)
{
    TRACE("EVENT --> aci_blue_initialized_event, reason code: 0x%X", Reason_Code);
}

//========================================================================
//=========================== Devices scan events ========================
//========================================================================

/**
  * @brief The LE Advertising Report event indicates that a Bluetooth device or multiple
Bluetooth devices have responded to an active scan or received some information
during a passive scan. The Controller may queue these advertising reports
and send information from multiple devices in one LE Advertising Report event.
  * @param Num_Reports Number of responses in this event.
  * Values:
  - 0x01
  * @param Advertising_Report See @ref Advertising_Report_t
  * @retval None
*/
void hci_le_advertising_report_event(uint8_t num_reports, Advertising_Report_t advertising_report[])
{
    TRACE("EVENT --> hci_le_advertising_report_event   nreports: %d", num_reports);
    for (int i = 0; i < num_reports; i++)
    {
        TRACE_PRINTFF("   Found device addr: ");
        for (int j = 0; j < 6; j++)
        {
            TRACE_PRINTF("%02X", advertising_report[i].Address[j]);
        }
        TRACE_PRINTF("  RSSI: %d", advertising_report[i].RSSI);

        TRACE_PRINTF("  Data(%d): [", advertising_report[i].Length_Data);
        for (int j = 0; j < advertising_report[i].Length_Data; j++)
        {
            TRACE_PRINTF("%02X ", advertising_report[i].Data[j]);
        }
        TRACE_PRINTF("]\n");

        memcpy(&test_addr.value, advertising_report[i].Address, sizeof(test_addr.value));
    }

    ble_stop_scan();
}

/**
 * @brief  This event is sent by the GAP to the upper layers when a procedure previously started has
been terminated by the upper layer or has completed for any other reason
 * @param  param See file bluenrg1_events.h.
 * @retval retVal See file bluenrg1_events.h.
*/
void aci_gap_proc_complete_event(uint8_t procedure_code, uint8_t status, uint8_t data_length, uint8_t data[])
{
    TRACE("EVENT --> aci_gap_proc_complete_event, procedure_code: 0x%x", procedure_code);
    state = STATE_CONNECT;
}

//============================================================================
// ============================= Connection events ===========================
//============================================================================

/**
  * @brief The LE Connection Complete event indicates to both of the Hosts forming the
connection that a new connection has been created. Upon the creation of the
connection a Connection_Handle shall be assigned by the Controller, and
passed to the Host in this event. If the connection establishment fails this event
shall be provided to the Host that had issued the LE_Create_Connection command.
This event indicates to the Host which issued a LE_Create_Connection
command and received a Command Status event if the connection
establishment failed or was successful.
The Master_Clock_Accuracy parameter is only valid for a slave. On a master,
this parameter shall be set to 0x00.
  * @param Status For standard error codes see Bluetooth specification, Vol. 2, part D. For proprietary error code refer to Error codes section
  * @param Connection_Handle Connection handle to be used to identify the connection with the peer device.
  * Values:
  - 0x0000 ... 0x0EFF
  * @param Role Role of the local device in the connection.
  * Values:
  - 0x00: Master
  - 0x01: Slave
  * @param Peer_Address_Type The address type of the peer device.
  * Values:
  - 0x00: Public Device Address
  - 0x01: Random Device Address
  * @param Peer_Address Public Device Address or Random Device Address of the peer
device
  * @param Conn_Interval Connection interval used on this connection.
Time = N * 1.25 msec
  * Values:
  - 0x0006 (7.50 ms)  ... 0x0C80 (4000.00 ms) 
  * @param Conn_Latency Slave latency for the connection in number of connection events.
  * Values:
  - 0x0000 ... 0x01F3
  * @param Supervision_Timeout Supervision timeout for the LE Link.
It shall be a multiple of 10 ms and larger than (1 + connSlaveLatency) * connInterval * 2.
Time = N * 10 msec.
  * Values:
  - 0x000A (100 ms)  ... 0x0C80 (32000 ms) 
  * @param Master_Clock_Accuracy Master clock accuracy. Only valid for a slave.
  * Values:
  - 0x00: 500 ppm
  - 0x01: 250 ppm
  - 0x02: 150 ppm
  - 0x03: 100 ppm
  - 0x04: 75 ppm
  - 0x05: 50 ppm
  - 0x06: 30 ppm
  - 0x07: 20 ppm
  * @retval None
*/
void hci_le_connection_complete_event(uint8_t status,
                                      uint16_t _connection_handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)
{
    TRACE("EVENT --> hci_le_connection_complete_event, Status: 0x%x   connection_handle: 0x%x", status, _connection_handle);
    connection_handle = _connection_handle;
    state = STATE_FIND_CHAR;
}

//=======================================================================================
// ============================= Read/write properties events ===========================
//=======================================================================================

void aci_gatt_disc_read_char_by_uuid_resp_event(uint16_t Connection_Handle, uint16_t Attribute_Handle, uint8_t Attribute_Value_Length, uint8_t Attribute_Value[]) 
{
    attr_handle = Attribute_Handle + 1;
    TRACE("aci_gatt_disc_read_char_by_uuid_resp_event, Attribute_Handle Handle: 0x%x\n", Attribute_Handle);
    state = STATE_READ_CHAR;       
}

/**
  * @brief This event is generated in response to a Read Request. See @ref aci_gatt_read_char_value.
  * @param Connection_Handle Connection handle related to the response
  * @param Event_Data_Length Length of following data
  * @param Attribute_Value The value of the attribute.
  * @retval None
*/
void aci_att_read_resp_event(uint16_t connection_handle,
                             uint8_t event_data_length,
                             uint8_t attribute_value[])
{
    TRACE_PRINTFF("EVENT --> aci_att_read_resp_event len: %d  [ ", event_data_length);
    for (int i = 0; i < event_data_length; i++) {
        TRACE_PRINTF("%02X ", attribute_value[i]);
    }
    TRACE_PRINTF(" ]\n");

    state = STATE_READ_CHAR;
}                             

//============================================================================
// ============================= Discover events ===========================
//============================================================================

/**
  * @brief This event is generated in response to a Read By Group Type Request. See
@ref aci_gatt_disc_all_primary_services.
  * @param Connection_Handle Connection handle related to the response
  * @param Attribute_Data_Length The size of each attribute data
  * @param Data_Length Length of Attribute_Data_List in octets
  * @param Attribute_Data_List Attribute Data List as defined in Bluetooth Core v4.1 spec. A sequence of attribute handle, end group handle, attribute value tuples: [2 octets for Attribute Handle, 2 octets End Group Handle, (Attribute_Data_Length - 4 octets) for Attribute Value]
  * @retval None
*/
void aci_att_read_by_group_type_resp_event(uint16_t Connection_Handle,
                                           uint8_t Attribute_Data_Length,
                                           uint8_t Data_Length,
                                           uint8_t Attribute_Data_List[])
{
    TRACE("EVENT --> aci_att_read_by_group_type_resp_event");
}                                           


/**
  * @brief This event is generated when an Error Response is received from the server. The error
response can be given by the server at the end of one of the GATT discovery procedures.
This does not mean that the procedure ended with an error, but this error event is part of the
procedure itself.
  * @param Connection_Handle Connection handle related to the response
  * @param Req_Opcode The request that generated this error response
  * @param Attribute_Handle The attribute handle that generated this error response
  * @param Error_Code The reason why the request has generated an error response (ATT error codes)
  * Values:
  - 0x01: Invalid handle
  - 0x02: Read not permitted
  - 0x03: Write not permitted
  - 0x04: Invalid PDU
  - 0x05: Insufficient authentication
  - 0x06: Request not supported
  - 0x07: Invalid offset
  - 0x08: Insufficient authorization
  - 0x09: Prepare queue full
  - 0x0A: Attribute not found
  - 0x0B: Attribute not long
  - 0x0C: Insufficient encryption key size
  - 0x0D: Invalid attribute value length
  - 0x0E: Unlikely error
  - 0x0F: Insufficient encryption
  - 0x10: Unsupported group type
  - 0x11: Insufficient resources
  * @retval None
*/
void aci_gatt_error_resp_event(uint16_t Connection_Handle,
                               uint8_t Req_Opcode,
                               uint16_t Attribute_Handle,
                               uint8_t Error_Code)
{
    //TRACE("EVENT --> aci_gatt_error_resp_event   Error_code: 0x%x  Attribute_Handle: 0x%x", Error_Code, Attribute_Handle);
}                               


/**
  * @brief This event is generated when a GATT client procedure completes either with error or
successfully.
  * @param Connection_Handle Connection handle related to the response
  * @param Error_Code Indicates whether the procedure completed with an error or was successful
  * Values:
  - 0x00: Success
  - 0x01: Unknown HCI Command
  - 0x02: Unknown Connection Identifier
  - 0x03: Hardware Failure
  - 0x04: Page Timeout
  - 0x05: Authentication Failure
  - 0x06: PIN or Key Missing
  - 0x07: Memory Capacity Exceeded
  - 0x08: Connection Timeout
  - 0x09: Connection Limit Exceeded
  - 0x0A: Synchronous Connection Limit to a Device Exceeded
  - 0x0B: ACL Connection Already Exists
  - 0x0C: Command Disallowed
  - 0x0D: Connection Rejected Due To Limited Resources
  - 0x0E: Connection Rejected Due To Security Reasons
  - 0x0F: Connection Rejected due to Unacceptable BD_ADDR
  - 0x10: Connection Accept Timeout Exceeded
  - 0x11: Unsupported Feature Or Parameter Value
  - 0x12: Invalid HCI Command Parameters
  - 0x13: Remote User Terminated Connection
  - 0x14: Remote Device Terminated Connection due to Low Resources
  - 0x15: Remote Device Terminated Connection due to Power Off
  - 0x16: Connection Terminated By Local Host
  - 0x17: Repeated Attempts
  - 0x18: Pairing Not Allowed
  - 0x19: Unknown LMP PDU
  - 0x1A: Unsupported Remote Feature / Unsupported LMP Feature
  - 0x1B: SCO Offset Rejected
  - 0x1C: SCO Interval Rejected
  - 0x1D: SCO Air Mode Rejected
  - 0x1E: Invalid LMP Parameters
  - 0x1F: Unspecified Error
  - 0x20: Unsupported LMP Parameter Value
  - 0x21: Role Change Not Allowed
  - 0x22: LMP Response Timeout / LL Response Timeout
  - 0x23: LMP Error Transaction Collision
  - 0x24: LMP PDU Not Allowed
  - 0x25: Encryption Mode Not Acceptable
  - 0x26: Link Key cannot be Changed
  - 0x27: Requested QoS Not Supported
  - 0x28: Instant Passed
  - 0x29: Pairing With Unit Key Not Supported
  - 0x2A: Different Transaction Collision
  - 0x2C: QoS Unacceptable Parameter
  - 0x2D: QoS Rejected
  - 0x2E: Channel Assessment Not Supported
  - 0x2F: Insufficient Security
  - 0x30: Parameter Out Of Mandatory Range
  - 0x32: Role Switch Pending
  - 0x34: Reserved Slot Violation
  - 0x35: Role Switch Failed
  - 0x36: Extended Inquiry Response Too Large
  - 0x37: Secure Simple Pairing Not Supported by Host
  - 0x38: Host Busy - Pairing
  - 0x39: Connection Rejected due to No Suitable Channel Found
  - 0x3A: Controller Busy
  - 0x3B: Unacceptable Connection Interval
  - 0x3C: Directed Advertising Timeout
  - 0x3D: Connection Terminated Due to MIC Failure
  - 0x3E: Connection Failed to be Established
  - 0x3F: MAC of the 802.11 AMP
  - 0x41: Failed
  - 0x42: Invalid parameters
  - 0x43: Busy
  - 0x44: Invalid length
  - 0x45: Pending
  - 0x46: Not allowed
  - 0x47: GATT error
  - 0x48: Address not resolved
  - 0x50: Invalid CID
  - 0x5A: CSRK not found
  - 0x5B: IRK not found
  - 0x5C: Device not found in DB
  - 0x5D: Security DB full
  - 0x5E: Device not bonded
  - 0x5F: Device in blacklist
  - 0x60: Invalid handle
  - 0x61: Invalid parameter
  - 0x62: Out of handles
  - 0x63: Invalid operation
  - 0x64: Insufficient resources
  - 0x65: Insufficient encryption key size
  - 0x66: Characteristic already exist
  - 0x82: No valid slot
  - 0x83: Short window
  - 0x84: New interval failed
  - 0x85: Too large interval
  - 0x86: Slot length failed
  - 0xFA: Flash read failed
  - 0xFB: Flash write failed
  - 0xFC: Flash erase failed
  * @retval None
*/
void aci_gatt_proc_complete_event(uint16_t Connection_Handle, uint8_t Error_Code)
{
    TRACE("EVENT --> aci_gatt_proc_complete_event   Error_code: 0x%x", Error_Code);
}
