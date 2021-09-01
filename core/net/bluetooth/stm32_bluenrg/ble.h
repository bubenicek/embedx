
#ifndef __BLE_H
#define __BLE_H

#ifndef CFG_BLE_WAITFOR_TMO
#define CFG_BLE_WAITFOR_TMO     3000
#endif

typedef struct
{
    uint8_t value[6];

} ble_addr_t;


typedef struct
{
    void (*scan_found_device)(ble_addr_t *addr, int rssi, uint8_t *data, int datasize);
    void (*scan_finished)(void);
    void (*error)(int errcode, const char *errmsg);

} ble_events_t;


/** Initialize BLE stack */
int ble_init(const ble_events_t *events);

/** Start BLE scan */
int ble_start_scan(void);

/** Stop BLE scan */
int ble_stop_scan(void);

/** Connect BLE device */
int ble_connect(ble_addr_t *addr);

/** Disconnect BLE device */
int ble_disconnect(void);

/** Find characteristic attribute by UUID */
int ble_find_attribute(const char *uuid128);

/** Read characteristic */
int ble_read_attribute(int attr_handle, void *buf, int bufsize);

/** Write characteristic */
int ble_write_attribute(int attr_handle, const void *buf, int bufsize);


#endif   // __BLE_H
