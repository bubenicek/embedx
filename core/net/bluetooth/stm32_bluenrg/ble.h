
#ifndef __BLE_H
#define __BLE_H

#ifndef CFG_BLE_WAITFOR_TMO
#define CFG_BLE_WAITFOR_TMO             3000
#endif

#ifndef CFG_BLE_NAMELEN
#define CFG_BLE_NAMELEN                 32
#endif

#define BLE_ADVER_TYPE_FLAGS            0x01
#define BLE_ADVER_SERVICES_LIST         0x03
#define BLE_ADVER_TYPE_SHORTLEN_NAME    0x08
#define BLE_ADVER_TYPE_COMPLETE_NAME    0x09


typedef struct
{
    uint8_t type;
    uint8_t value[6];

} ble_addr_t;


typedef struct
{
    char name[CFG_BLE_NAMELEN];
    ble_addr_t addr;
    int rssi;

} ble_device_t;


typedef struct
{
    void (*scan_found_device)(ble_device_t *dev, uint8_t *data, int datasize);
    void (*scan_finished)(void);
    void (*error)(int errcode, const char *errmsg);

} ble_events_t;


/** Initialize BLE stack */
int ble_init(const ble_events_t *events);

/** Reset BLE host */
int ble_reset(void);

/** Start BLE scan */
int ble_start_scan(void);

/** Stop BLE scan */
int ble_stop_scan(void);

/** Connect BLE device */
int ble_connect(ble_addr_t *addr);

/** Disconnect BLE device */
int ble_disconnect(void);

/** Check for ble connection */
bool ble_is_connected(void);

/** Find characteristic attribute by UUID */
int ble_find_attribute(const char *uuid128);

/** Read characteristic */
int ble_read_attribute(int attr_handle, void *buf, int bufsize);

/** Write characteristic */
int ble_write_attribute(int attr_handle, const void *buf, int bufsize);

/** Convert address to string */
const char *ble_addr2str(ble_addr_t *addr);

/** Compare two address, return 0 if equal else -1 */
int ble_addrcmp(ble_addr_t *a1, ble_addr_t *a2);

/** Copy address */
void ble_addrcpy(ble_addr_t *dest, ble_addr_t *src);

#endif   // __BLE_H
