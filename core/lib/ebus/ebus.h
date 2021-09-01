
#ifndef __EBUS_H
#define __EBUS_H

#include "ebus_cmd.h"

#define EBUS_UART_BAUDRATE                2400
#define EBUS_SYNC                         0xAA
#define EBUS_ACK                          0x00
#define EBUS_NACK                         0xFF

#define CFG_EBUS_VALID_SYNC_COUNT         10
#define CFG_EBUS_WAITFOR_SYNC_TIMEOUT     1000
#define CFG_EBUS_PACKETLEN                16
#define CFG_EBUS_TXRX_RETRY               3


/** EBUS address type */
typedef uint8_t ebus_addr_t;

/** EBUS request packet */
typedef struct
{
   uint8_t src;
   uint8_t dst;
   uint8_t cmd_primary;
   uint8_t cmd_second;
   uint8_t datalen;
   uint8_t data[CFG_EBUS_PACKETLEN];

}  __attribute__ ((__packed__)) ebus_request_packet_t;

#define EBUS_REQUEST_PACKET_SIZE(_pkt) (_pkt->datalen + 5)


/** EBUS response packet */
typedef struct
{
   uint8_t datalen;
   uint8_t data[CFG_EBUS_PACKETLEN];

}  __attribute__ ((__packed__)) ebus_response_packet_t;


/** EBUS link connection */
typedef struct
{
   hal_uart_t uart;
   ebus_addr_t src_addr;

} ebus_t;


/** Initialize ebus */
int ebus_init(ebus_t *ebus, hal_uart_t uart, ebus_addr_t src_addr);

/** Send request and receive response */
int ebus_sendrecv_packet(ebus_t *ebus, ebus_request_packet_t *req, ebus_response_packet_t *resp);

/** Send command and receive response */
int ebus_sendrecv_command(ebus_t *ebus, const char *name, const ebus_value_t *param, ebus_value_t *value);

#endif   // __EBUS_H
