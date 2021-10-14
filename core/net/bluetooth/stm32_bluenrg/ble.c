
#include "system.h"

#include "uuid.h"

#include "osal.h"
#include "hci.h"
#include "bluenrg1_hci_le.h"
#include "bluenrg1_gap_aci.h"
#include "bluenrg1_hal_aci.h"
#include "bluenrg1_gatt_aci.h"
#include "bluenrg1_events.h"

#include "ble.h"

TRACE_TAG(ble);
#if !ENABLE_TRACE_BLE
#include "trace_undef.h"
#endif


// Types:

typedef enum
{
    BLE_REQUEST_NONE,
    BLE_REQUEST_SCAN,
    BLE_REQUEST_CONNECT,
    BLE_REQUEST_DISCONNECT,
    BLE_REQUEST_FIND_ATTRIBUTE,
    BLE_REQUEST_READ_ATTRIBUTE,
    BLE_REQUEST_WRITE_ATTRIBUTE

} ble_request_type_t;


typedef struct
{
    ble_request_type_t type;
    osSemaphoreId sem;
    bool pending;
    bool sync;

    // Response
    struct 
    {
        int code;

        union 
        {
            uint16_t connection_handle;
            uint16_t attribute_handle;

            struct
            {
                uint8_t *ptr;
                int size;

            } buf;

        } data;

    } result;   

} ble_request_t;


// Prototypes:
static void bluenrg_thread(void *arg);
static const osThreadDef(BLUENRG_THREAD, bluenrg_thread, 0, 0, 1024);

// Locals:
static const ble_events_t *events = NULL;
static ble_request_t request;
static uint16_t connection_handle;


/** Init a BlueNRG device */
static int ble_device_init(void)
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

    return 0;
}

static int request_start(ble_request_type_t type, bool sync)
{
    if (request.pending)
    {
        TRACE_ERROR("BLE request is already pending");
        return -1;
    }

    request.type = type;
    request.pending = true;
    request.sync = sync;

    if (request.sync)
    {
        if (osSemaphoreWait(request.sem, CFG_BLE_WAITFOR_TMO) != osOK)
        {
            TRACE_ERROR("Wait for request: %d response failed", type);
            return -1;
        }

        if (request.result.code != 0)
            return -1;
    }

    return 0;
}

static int request_finish(int rescode)
{
    if (request.pending)
    {
        if (request.sync)
        {
            osSemaphoreRelease(request.sem);
        }

        request.result.code = rescode;
        request.pending = false;
        request.type = BLE_REQUEST_NONE;
    }

    return 0;
}

static void bluenrg_thread(void *arg)
{
    TRACE("bluenrg thread started");

    while (1)
    {
        BTLE_StackTick();
        osDelay(25);
    }
}


/** Initialize BLE stack */
int ble_init(const ble_events_t *_events)
{
    events = _events;

    memset(&request, 0, sizeof(request));

    if ((request.sem = osSemaphoreCreate(NULL, 1)) == NULL)
    {
        TRACE_ERROR("Create semaphore");
        return -1;
    }

    if (osSemaphoreWait(request.sem, osWaitForever) != osOK)
    {
        TRACE_ERROR("Sem wait");
        return -1;
    }

    // Initialize UART
    if (hal_uart_init(BLUENRG_UART) != 0)
    {
        TRACE_ERROR("Init uart failed");
        return -1;
    }

    // Initialze BLE stack
    if (BlueNRG_Stack_Initialization() != 0)
    {
        TRACE_ERROR("BlueNRG_Stack_Initialization failed");
        return -1;
    }

    // Start receive events thread
    if (osThreadCreate(osThread(BLUENRG_THREAD), NULL) == 0)
    {
        TRACE_ERROR("Create bluenrg thread failed");
        return -1;
    }

    // Initialize BLE device
    if (ble_device_init() != 0)
    {
        TRACE_ERROR("BLE device init failed");
        return -1;
    }

    TRACE("Init");

    return 0;
}

/** Start BLE scan */
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

    return request_start(BLE_REQUEST_SCAN, false);
}

/** Stop BLE scan */
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

/** Connect BLE device */
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

    // Wait for request
    if (request_start(BLE_REQUEST_CONNECT, true) != 0)
    {
        TRACE_ERROR("Connect failed");
        return -1;
    }

    connection_handle = request.result.data.connection_handle;

    return 0;
}

/** Disconnect BLE device */
int ble_disconnect(void)
{
    // TODO:
    return 0;
}

/** Find characteristic attribute by UUID */
int ble_find_attribute(const char *uuid128)
{
    UUID_t uuid;
    uint8_t temp[16];
    
    uuid128_parse(uuid128, temp, 16);

    // Revert bytes order
    for (int i = 0; i < 16; i++) {
        uuid.UUID_128[15-i] = temp[i];
    }

    // Send request
    int status = aci_gatt_disc_char_by_uuid(connection_handle, 0x0001, 0xFFFF, UUID_TYPE_128, &uuid);
    if (status != BLE_STATUS_SUCCESS) {
        TRACE_ERROR("aci_gatt_disc_char_by_uuid failed: 0x%x", status);
        return -1;
    }
    TRACE("aci_gatt_disc_char_by_uuid --> SUCCESS");

    // Wait for response
    if (request_start(BLE_REQUEST_FIND_ATTRIBUTE, true) != 0)
    {
        TRACE_ERROR("Find attribute failed");
        return -1;
    }

    return request.result.data.attribute_handle;
}

/** Read characteristic */
int ble_read_attribute(int attr_handle, void *buf, int bufsize)
{
    int status = aci_gatt_read_char_value(connection_handle, attr_handle + 1);   // Read = +1
    if (status != BLE_STATUS_SUCCESS) 
    {
        TRACE_ERROR("aci_gatt_read_char_value failed: 0x%x", status);
        return -1;
    }
    TRACE("aci_gatt_read_char_value attr_handle: 0x%x--> SUCCESS", attr_handle);

    request.result.data.buf.ptr = buf;
    request.result.data.buf.size = bufsize;

    // Wait for response
    if (request_start(BLE_REQUEST_READ_ATTRIBUTE, true) != 0)
    {
        TRACE_ERROR("Read attribute failed");
        return -1;
    }

    return request.result.data.buf.size;
}

/** Write characteristic */
int ble_write_attribute(int attr_handle, const void *buf, int bufsize)
{
    // TODO:
    return 0;
}

/** Convert address to string */
const char *ble_addr2str(ble_addr_t *addr)
{
    static char str[20];
    int pos = 0;

    for (int i = 0; i < 6; i++) {
        pos += sprintf(&str[pos], "%02X", addr->value[i]);
    }

    return str;
}

/** Compare two address, return 0 if equal else -1 */
int ble_addrcmp(ble_addr_t *a1, ble_addr_t *a2)
{
    return memcmp(a1->value, a2->value, sizeof(ble_addr_t));
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
    char devname[255];

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

        *devname = '\0';

        if (advertising_report[i].Length_Data > 0)
        {
            switch(advertising_report[i].Data[1])
            {
                case BLE_ADVER_TYPE_SHORTLEN_NAME:
                case BLE_ADVER_TYPE_COMPLETE_NAME:
                    strncpy(devname, &advertising_report[i].Data[2], advertising_report[i].Data[0]-1);
                    break;
            }
        }

        if (events != NULL && events->scan_found_device != NULL)
        {
            events->scan_found_device((ble_addr_t *)&advertising_report[i].Address, devname, advertising_report[i].RSSI, advertising_report[i].Data, advertising_report[i].Length_Data);
        }
    }
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

    if (request.type == BLE_REQUEST_SCAN && request.pending) 
    {
        if (events != NULL && events->scan_finished != NULL) 
        {
            events->scan_finished();
        }        

        request_finish(0);
    }
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
                                      uint16_t connection_handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)
{
    TRACE("EVENT --> hci_le_connection_complete_event, Status: 0x%x   connection_handle: 0x%x", status, connection_handle);
    
    request.result.data.connection_handle = connection_handle;
    request_finish(0);
}

//=======================================================================================
// ============================= Read/write properties events ===========================
//=======================================================================================

void aci_gatt_disc_read_char_by_uuid_resp_event(uint16_t Connection_Handle, uint16_t Attribute_Handle, uint8_t Attribute_Value_Length, uint8_t Attribute_Value[]) 
{
    TRACE("aci_gatt_disc_read_char_by_uuid_resp_event, Attribute_Handle Handle: 0x%x\n", Attribute_Handle);

    request.result.data.attribute_handle = Attribute_Handle;
    request_finish(0);
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

    if (request.result.data.buf.size < event_data_length)
    {
        TRACE_ERROR("Output read buffer overflow");
        request_finish(-1);
    }
    else
    {
        request.result.data.buf.size = event_data_length;
        memcpy(request.result.data.buf.ptr, attribute_value, event_data_length); 
        request_finish(0);
    }
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
    request_finish(Error_Code);
}
