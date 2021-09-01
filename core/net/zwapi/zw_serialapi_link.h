
#ifndef __ZW_SERIALAPI_LINK_H
#define __ZW_SERIALAPI_LINK_H

#include <stdint.h>
#include <stdbool.h>
#include <cmsis_os.h>         // RTOS API

#include "zw_timer.h"

// Serial communication definitions
#define ZW_SOF                         0x01  // Start Of Frame
#define ZW_ACK                         0x06  // Acknowledge successfull frame reception
#define ZW_NAK                         0x15  // Not Acknowledge successfull frame reception - please retransmit...
#define ZW_CAN                         0x18  // Frame received (from host) was dropped - waiting for ACK
#define ZW_SYN                         0x16  // HOST Sync request

// Frame types
#define ZW_REQUEST                     0x00
#define ZW_RESPONSE                    0x01

#define ZW_FRAME_LENGTH_MIN            3
#define ZW_FRAME_LENGTH_MAX            180

// ACK Timeout is 1500 ms to allow HOST system to react - module transmits and waits
// max 1500ms before either starting a retransmit or simply drops the frame
#define ZW_RX_ACK_TIMEOUT              1500

// Receive byte inframe timeout is 150 ms to allow for byte not being fetched
// "instantaneously" from receive buffer after physically being buffered by the
// interrupt routine - this also includes HOST system byte transmit delays
#define ZW_RX_DATA_TIMEOUT             1500

// Response timeout
#define ZW_RX_RESPONSE_TIMEOUT         1500

#define IDX_LENGTH                     0
#define IDX_TYPE                       1

#define ZW_LINK_RX_FRAMES_SIZE         16
#define ZW_LINK_TX_RETRY_COUNT         3


/** RX states */
typedef enum
{
  ZW_SERIALAPI_LINK_RX_STATE_SOF,
  ZW_SERIALAPI_LINK_RX_STATE_LEN,
  ZW_SERIALAPI_LINK_RX_STATE_TYPE,
  ZW_SERIALAPI_LINK_RX_STATE_CMD,
  ZW_SERIALAPI_LINK_RX_STATE_DATA,
  ZW_SERIALAPI_LINK_RX_STATE_CRC

} zw_serialapi_rx_state_e;


/** TX states */
typedef enum
{
  ZW_SERIALAPI_LINK_TX_STATE_IDLE,
  ZW_SERIALAPI_LINK_TX_STATE_ACK,
  ZW_SERIALAPI_LINK_TX_STATE_DONE,
  ZW_SERIALAPI_LINK_TX_STATE_ERROR

} zw_serialapi_tx_state_e;


/** ZW frame */
typedef struct
{
   uint8_t length;
   uint8_t type;
   uint8_t cmd;
   uint8_t data[ZW_FRAME_LENGTH_MAX];

} zw_serialapi_frame_t;

#define ZW_FRAME_DATALEN(_f) ((_f)->length - 3)

/**
 * ZW link object
 */
typedef struct
{
   osThreadId thread_id;

   zw_serialapi_tx_state_e tx_state;
   osTimerId ack_timer;
   osSemaphoreId txframe_sem;

   struct
   {
      uint8_t type;
      uint8_t cmd;
      uint8_t *data;
      uint8_t datalen;
      int retry_cnt;

   } tx_frame;

   zw_serialapi_rx_state_e rx_state;
   osTimerId data_timer;

   uint8_t *rxbuf;
   int rxbuf_len;
   uint8_t rx_crc;

   struct
   {
      osMessageQId queue_id;
      zw_serialapi_frame_t buffer[ZW_LINK_RX_FRAMES_SIZE];
      uint16_t head;
      uint32_t nerr;
      int count;

   } rx_frames;

} zw_serialapi_link_t;


/** Open ZW link */
int zw_serialapi_link_open(zw_serialapi_link_t *link);

/** Close ZW link */
int zw_serialapi_link_close(zw_serialapi_link_t *link);

/** Transmit frame via serial port by adding SOF, Len, Type, cmd and Chksum. */
int zw_serialapi_link_send(zw_serialapi_link_t *link, uint8_t type, uint8_t cmd, uint8_t *data, int datalen);

/** Receive frame */
int zw_serialapi_link_recv(zw_serialapi_link_t *link, zw_serialapi_frame_t **frame);


#endif // __ZW_SERIALAPI_LINK_h
