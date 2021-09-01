#include "system.h"
#include "ble.h"

TRACE_TAG(bletest);

#define UUID_CHAR_BATTERY       "00002a19-0000-1000-8000-00805f9b34fb"


static bool scan_pending;
static ble_addr_t test_addr;


/** Found device callback */
static void ble_scan_found_device_callback(ble_addr_t *addr, int rssi,  uint8_t *data, int datasize)
{
    TRACE_PRINTFF("Found device: ");
    for (int i = 0; i < 6; i++) {
        TRACE_PRINTF("%02X", addr->value[i]);
    }
    TRACE_PRINTF("\n");

    test_addr = *addr;

    // Stop scanning
    ble_stop_scan();
}

/** Scan finished callback */
static void ble_scan_finished_callback(void)
{
    TRACE("Scan finished");
    scan_pending = false;
}

/** Error callback */
static void ble_error_callback(int errcode, const char *errmsg)
{
    TRACE_ERROR("BLE error: %d  %s", errcode, errmsg);
}


int bletest(void)
{
    ble_events_t events = {
        .scan_found_device = ble_scan_found_device_callback,
        .scan_finished = ble_scan_finished_callback,
        .error = ble_error_callback
    };

    int attr_handle;
    uint8_t value;

    // Initialize BLE stack
    if (ble_init(&events) != 0)
    {
        TRACE_ERROR("ble init failed");
        return -1;
    }

    // Start scan
    if (ble_start_scan() != 0) 
    {
        TRACE_ERROR("Start scan failed");
        return -1;
    }

    // Wait for scan finished
    scan_pending = true;
    while(scan_pending) {
        osDelay(1000);
    }

    // Connect to device
    if (ble_connect(&test_addr) != 0)
    {
        TRACE_ERROR("Connect failed");
        return -1;
    }

    TRACE("Connected");

    // Get handle of characteristic
    if ((attr_handle = ble_find_attribute(UUID_CHAR_BATTERY)) < 0)
    {
        TRACE_ERROR("Battery characteristics find failed");
        return -1;
    }
    TRACE("%s -> attr_handle: 0x%x", UUID_CHAR_BATTERY, attr_handle);

    // Read attribute value
    while(1)
    {
        if (ble_read_attribute(attr_handle, &value, sizeof(value)) != sizeof(value))
        {
            TRACE_ERROR("Read attribute failed");
            return -1;
        }

        TRACE("Attribute value: 0x%X (%d)", value, value);

        osDelay(1000);
    }

    return 0;
}
