/**
 * \file zw_serialapi.c    \bried ZW Serial API
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "zw_config.h"
#include "zw_serialapi.h"

#define TRACE_TAG    "zw_api"

#if !ENABLE_TRACE_ZW_SERIALAPI
#undef TRACE
#define TRACE(...)
#endif

#define SEQNO(_sapi) \
({ \
  uint8_t __res; \
  (_sapi)->seqno++; \
  if (!(_sapi)->seqno) \
      (_sapi)->seqno++; \
   __res = (_sapi)->response.seqno = (_sapi)->seqno; \
   __res; \
})

// Prototypes:
static void zw_serialapi_thread(const void *param);


/** Serial API initialization */
int zw_serialapi_init(zw_serialapi_t *sapi)
{
   const osThreadDef(ZW_SAPI, zw_serialapi_thread, ZW_SERIALAPI_THREAD_PRIORITY, 0, ZW_SERIALAPI_THREAD_STACK_SIZE);
   const osMessageQDef(rxQueue, ZW_LINK_RX_FRAMES_SIZE, zw_serialapi_frame_t *);
   const osMessageQDef(rxCmdQueue, ZW_LINK_RX_FRAMES_SIZE, zw_serialapi_cmd_frame_data_t *);

   memset(sapi, 0, sizeof(zw_serialapi_t));

   // Set options
   sapi->options.rx_timeout = ZW_RX_TIMEOUT;
   sapi->options.tx_options = ZW_TX_OPTIONS;

   sapi->api_mutex = osMutexCreate(NULL);
   if (sapi->api_mutex == NULL)
   {
      TRACE_ERROR("Create API mutex");
      goto fail_api_mutex;
   }

   // Create rx cmd msg queue
   sapi->rx_cmd_queue_id = osMessageCreate(osMessageQ(rxCmdQueue), 0);
   if ( sapi->rx_cmd_queue_id == NULL)
   {
      TRACE_ERROR("Create rx cmd queue");
      goto fail_create_rx_cmd_queue;
   }

   // Create response msg queue
   sapi->response.queue_id = osMessageCreate(osMessageQ(rxQueue), 0);
   if ( sapi->response.queue_id == NULL)
   {
      TRACE_ERROR("Create response queue");
      goto fail_create_rx_queue;
   }

   if (zw_serialapi_link_open(&sapi->zwlink) != 0)
   {
      TRACE_ERROR("Open zw link");
      goto fail_link_open;
   }

   // Create Start thread
   sapi->thread_id = osThreadCreate(osThread(ZW_SAPI), sapi);
   if (sapi->thread_id == NULL)
   {
      TRACE_ERROR("Start requests thread");
      goto fail_create_thread;
   }

   TRACE("ZW serial API init, eeprom type: %d", ZW_EEPROM_TYPE);

   return 0;

fail_create_thread:
   VERIFY(zw_serialapi_link_close(&sapi->zwlink) == 0);
fail_link_open:
fail_create_rx_queue:
fail_create_rx_cmd_queue:
   VERIFY(osMutexDelete(sapi->api_mutex) == osOK);
fail_api_mutex:
   return -1;
}

/** Serial API finalization */
int zw_serialapi_deinit(zw_serialapi_t *sapi)
{
   VERIFY(osThreadTerminate(sapi->thread_id) == osOK);
   VERIFY(zw_serialapi_link_close(&sapi->zwlink) == 0);
   VERIFY(osMutexDelete(sapi->api_mutex) == osOK);

   TRACE("ZW serial API deinit");

   return 0;
}

/** Thread for executing incomming requests */
static void zw_serialapi_thread(const void *param)
{
   zw_serialapi_t *sapi = (zw_serialapi_t *)param;
   zw_serialapi_frame_t *rx_frame;

   TRACE("SerialAPI requests thread is running ...");

   while(1)
   {
      // Wait until frame is not received
      if (zw_serialapi_link_recv(&sapi->zwlink, &rx_frame) != 0)
      {
         TRACE_ERROR("Recv frame");
         continue;
      }
#if ENABLE_TRACE_ZW_SERIALAPI_DATA
      int ix;
      TRACE_PRINTFF("Recv %s cmd: 0x%02X length: %d <- ", rx_frame->type == ZW_RESPONSE ? "RESPONSE" : "REQUEST ", rx_frame->cmd, ZW_FRAME_DATALEN(rx_frame));
      for (ix = 0; ix < ZW_FRAME_DATALEN(rx_frame); ix++)
      {
         TRACE_PRINTF("%02X ", rx_frame->data[ix]);
      }
      TRACE_PRINTF(TRACE_NL);
#endif
      switch(rx_frame->type)
      {
         case ZW_RESPONSE:
         {
            if (osMessagePut(sapi->response.queue_id, (uint32_t)rx_frame, osWaitForever) != osOK)
            {
               TRACE_ERROR("Responses queue is full");
            }
         }
         break;

         case ZW_REQUEST:
         {
            TRACE("Waiting response cmd: 0x%X", sapi->response.cmd);

            if (rx_frame->cmd == sapi->response.cmd)
            {
               TRACE("Add to response queue");

               // Request as response
               if (rx_frame->data[0] == sapi->response.seqno)
               {
                  // Invalidate response
                  sapi->response.cmd = 0;
                  sapi->response.seqno = 0;

                  if (osMessagePut(sapi->response.queue_id, (uint32_t)rx_frame, osWaitForever) != osOK)
                  {
                     TRACE_ERROR("Responses queue is full");
                  }
               }
               else
               {
                  TRACE_ERROR("Recv not expected SEQ: %d != %d", sapi->response.seqno, rx_frame->data[0]);
               }
            }
            else
            {
               // Request
               TRACE("Add to request queue");

               if (osMessagePut(sapi->rx_cmd_queue_id,  (uint32_t)rx_frame, osWaitForever) != osOK)
               {
                  TRACE_ERROR("RX CMD frames queue is full");
               }
            }
         }
         break;

         default:
            TRACE_ERROR("Unknown type of frame: %d", rx_frame->type);
      }
   }
}

/** Recv command data frame (incomming commands) */
int zw_serialapi_recv_request(zw_serialapi_t *sapi, zw_serialapi_frame_t **frame, uint32_t timeout)
{
   osEvent event;

   // Wait for receive frame
   event = osMessageGet(sapi->rx_cmd_queue_id, timeout);
   if (event.status != osEventMessage)
   {
      if (event.status == osEventTimeout)
      {
         return -2;
      }
      else
      {
         return -1;
      }
   }

   *frame = event.value.p;

   return 0;
}

static int zw_serialapi_send_request(zw_serialapi_t *sapi, uint8_t cmd, uint8_t *data, int datalen, zw_serialapi_frame_t **frame)
{
   osEvent event;

   sapi->response.cmd = cmd;

   // Send request
   if (zw_serialapi_link_send(&sapi->zwlink, ZW_REQUEST, cmd, data, datalen) != 0)
   {
      TRACE_ERROR("Send ZW_REQUEST cmd: 0x%X", cmd);
      return -1;
   }

#if ENABLE_TRACE_ZW_SERIALAPI_DATA
   int ix;
   TRACE_PRINTFF("Send REQUEST  cmd: 0x%02X length: %d -> ", cmd, datalen);
   for (ix = 0; ix < datalen; ix++)
   {
      TRACE_PRINTF("%02X ", data[ix]);
   }
   TRACE_PRINTF(TRACE_NL);
#endif

   // Wait for receive response
   event = osMessageGet(sapi->response.queue_id, ZW_RX_RESPONSE_TIMEOUT);
   if (event.status != osEventMessage)
   {
      TRACE_ERROR("Wait for response cmd: 0x%X timeouted", cmd);
      return -1;
   }

   *frame = event.value.p;

   // Check received response cmd
   if ((*frame)->cmd != cmd)
   {
      TRACE_ERROR("Recv not expected response cmd: 0x%X", (*frame)->cmd);
      return -1;
   }

   return 0;
}

static int __zw_serialapi_send_data_ext(zw_serialapi_t *sapi, uint8_t node_id, uint8_t *data, uint8_t datalen, uint8_t tx_options, uint16_t timeout)
{
   int index = 0;
   osEvent event;
   zw_serialapi_frame_t *rx_frame;
   uint8_t prev_expect_cmd = sapi->response.cmd;;

   sapi->tx_frame.data[index++] = node_id;
   sapi->tx_frame.data[index++] = datalen;
   while (datalen > 0)
   {
      sapi->tx_frame.data[index++] = *data++;
      datalen--;
   }
   sapi->tx_frame.data[index++] = tx_options;
   sapi->tx_frame.data[index++] = SEQNO(sapi);

   if (node_id != 0xFF)
   {
      // Send request and wait for response
      if (zw_serialapi_send_request(sapi, FUNC_ID_ZW_SEND_DATA, sapi->tx_frame.data, index, &rx_frame) != 0)
      {
         goto fail;
      }

      // Wait for receive request as response
      event = osMessageGet(sapi->response.queue_id, !timeout ? osWaitForever : timeout);
      if (event.status != osEventMessage)
      {
         TRACE_ERROR("Wait for request response timeouted: %d", timeout);
         goto fail;
      }
      rx_frame = event.value.p;

      if (rx_frame->data[1] != TRANSMIT_COMPLETE_OK)
      {
         TRACE_ERROR("Recv reponse status: %d", rx_frame->data[1]);
         goto fail;
      }

      // Set previous expected request as response
      sapi->response.cmd = prev_expect_cmd;

      return 0;
   }
   else
   {
      // Send broadcast data without waiting
      return zw_serialapi_send_request(sapi, FUNC_ID_ZW_SEND_DATA, sapi->tx_frame.data, index, &rx_frame);
   }

fail:
   // Set previous expected request as response
   sapi->response.cmd = prev_expect_cmd;
   return -1;
}

//
// Serial API fnctions impl.
//

/**
 * Transmit data buffer to a single ZW-node or all ZW-nodes (broadcast).
 */
int zw_serialapi_send_data_ext(zw_serialapi_t *sapi, uint8_t node_id, uint8_t *data, uint8_t datalen, uint8_t tx_options, uint16_t timeout)
{
   int res;

   osMutexWait(sapi->api_mutex, osWaitForever);
   res = __zw_serialapi_send_data_ext(sapi, node_id, data, datalen, tx_options, timeout);
   osMutexRelease(sapi->api_mutex);

   return res;
}

/** Send data and wait for application response data */
int zw_serialapi_sendrecv_data_ext(zw_serialapi_t *sapi, uint8_t node_id, uint8_t *out_data, uint8_t out_datalen,
                                   uint8_t tx_options, uint16_t timeout, uint8_t *in_data, uint8_t *in_datalen)
{
   osEvent event;
   zw_serialapi_frame_t *rx_frame;

   ASSERT(node_id != 0xFF);

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Set expected request as response
   sapi->response.cmd = FUNC_ID_APPLICATION_COMMAND_HANDLER;

   if (__zw_serialapi_send_data_ext(sapi, node_id, out_data, out_datalen, tx_options, timeout) != 0)
   {
      goto fail;
   }

   // Wait for receive response
   event = osMessageGet(sapi->response.queue_id, !timeout ? osWaitForever : timeout);
   if (event.status != osEventMessage)
   {
      TRACE_ERROR("Wait for request response timeouted: %d", timeout);
      goto fail;
   }
   rx_frame = event.value.p;

   if (*in_datalen >= ZW_FRAME_DATALEN(rx_frame))
   {
      memcpy(in_data, &rx_frame->data[3], ZW_FRAME_DATALEN(rx_frame) - 3);
      *in_datalen = ZW_FRAME_DATALEN(rx_frame) - 3;
   }
   else
   {
      TRACE_ERROR("Received in_data buffer is overflow");
      goto fail;
   }

   osMutexRelease(sapi->api_mutex);

   return 0;

fail:
   osMutexRelease(sapi->api_mutex);
   return -1;
}

/** Test node connection - ping */
int zw_serialapi_ping_node(zw_serialapi_t *sapi, uint8_t node_id, uint16_t timeout)
{
   uint8_t ping_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   return zw_serialapi_send_data_ext(sapi, node_id, ping_data, sizeof(ping_data), sapi->options.tx_options, timeout);
}

/**
 * Get the Z-Wave library basis version
 * @param pBuf			OUT Pointer to buffer where version string will be copied to. Buffer must be at least 14 bytes long.
 */
int zw_serialapi_get_version(zw_serialapi_t *sapi, uint8_t *outbuf, uint8_t outbuf_size)
{
   int ret;
   zw_serialapi_frame_t *rx_frame;

   ASSERT(outbuf_size >= 12);

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   if (zw_serialapi_send_request(sapi, FUNC_ID_ZW_GET_VERSION, NULL, 0, &rx_frame) != 0)
   {
      goto fail;
   }

   // Check response result
   if (memcmp(rx_frame->data, "Z-Wave", 6) != 0)
   {
      goto fail;
   }

   ret = rx_frame->data[12];
   memcpy(outbuf, rx_frame->data, 12);
   outbuf[12] = '\0';

   osMutexRelease(sapi->api_mutex);

   return (ret == 1) ? 0 : -1;

fail:
   osMutexRelease(sapi->api_mutex);
   return -1;
}

/**
 *	Get Serial API initialization data from remote side (Enhanced Z-Wave module)
 *
 *	@param ver				OUT  Remote sides Serial API version
 * @param capabilities	OUT  Capabilities flag (GET_INIT_DATA_FLAG_xxx)
 *	@param nodes_list		OUT  Bitmask list with nodes known by Z-Wave module
 * @param nodes_list_size OUT  Number of bytes in nodesList
 *
 * Capabilities flag:
 * Bit 0: 0 = Controller API; 1 = Slave API
 * Bit 1: 0 = Timer functions not supported; 1 = Timer functions supported.
 * Bit 2: 0 = Primary Controller; 1 = Secondary Controller
 * Bit 3: 0 = Not SUC; 1 = This node is SUC (static controller only)
 * Bit 3-7: reserved
 * Timer functions are: TimerStart, TimerRestart and TimerCancel.
 *
 * @return 0 if success else -1 if any error
 */
int zw_serialapi_get_init_data(zw_serialapi_t *sapi, uint8_t *ver, uint8_t *capabilities, uint8_t *nodes_list, uint8_t *nodes_list_size)
{
   int res, ix;
   zw_serialapi_frame_t *rx_frame;

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_SERIAL_API_GET_INIT_DATA, NULL, 0, &rx_frame);
   if (res == 0)
   {
      *ver = rx_frame->data[0];
      *capabilities = rx_frame->data[1];

      ASSERT(*nodes_list_size >= rx_frame->data[2]);
      *nodes_list_size = rx_frame->data[2];

      for (ix = 0; ix < *nodes_list_size; ix++)
      {
         nodes_list[ix] = rx_frame->data[3 + ix];
      }
   }

   osMutexRelease(sapi->api_mutex);

   return res;
}

/**
 * Copy the Node's current protocol information from the non-volatile memory.
 */
int zw_serialapi_get_node_protocol_info(zw_serialapi_t *sapi, uint8_t node_id, zw_node_info_t *node_info)
{
   int res;
   zw_serialapi_frame_t *rx_frame;

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO, &node_id, 1, &rx_frame);
   if (res == 0)
   {
      node_info->capability = rx_frame->data[0];
      node_info->security = rx_frame->data[1];
      node_info->reserved = rx_frame->data[2];
      node_info->node_type.basic = rx_frame->data[3];
      node_info->node_type.generic = rx_frame->data[4];
      node_info->node_type.specific = rx_frame->data[5];
   }

   osMutexRelease(sapi->api_mutex);

   return res;
}

/**
 * Copy the Home-ID and Node-ID to the specified RAM addresses
 */
int zw_serialapi_memory_get_id(zw_serialapi_t *sapi, uint32_t *home_id, uint8_t *node_id)
{
   int res;
   zw_serialapi_frame_t *rx_frame;

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_MEMORY_GET_ID, NULL, 0, &rx_frame);
   if (res == 0)
   {
      *home_id = rx_frame->data[0] << 24;
      *home_id |= rx_frame->data[1] << 16;
      *home_id |= rx_frame->data[2] << 8;
      *home_id |= rx_frame->data[3];
      *node_id = rx_frame->data[4];
   }

   osMutexRelease(sapi->api_mutex);

   return res;
}

int zw_serialapi_memory_set_id(zw_serialapi_t *sapi, uint32_t home_id, uint8_t node_id)
{
   int res;
   zw_serialapi_frame_t *rx_frame;
   uint8_t data[10];
   int idx = 0;

   osMutexWait(sapi->api_mutex, osWaitForever);

   data[idx++] = (home_id >> 24) & 0xFF;
   data[idx++] = (home_id >> 16) & 0xFF;
   data[idx++] = (home_id >> 8) & 0xFF;
   data[idx++] = home_id & 0xFF;
   data[idx++] = node_id;

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_STORE_HOMEID, data, idx, &rx_frame);

   osMutexRelease(sapi->api_mutex);

   return res;
}


/** Get the currently registered SUC node ID */
int zw_serialapi_get_SUC_node_id(zw_serialapi_t *sapi, uint8_t *node_id)
{
   int res;
   zw_serialapi_frame_t *rx_frame;

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_ZW_GET_SUC_NODE_ID, NULL, 0, &rx_frame);
   if (res == 0)
   {
      *node_id = rx_frame->data[0];
   }

   osMutexRelease(sapi->api_mutex);

   return res;
}

/**
 *	Get Controller Capabilities
 *
 * Capabilities flags:
 * CONTROLLER_CAPABILITIES_IS_SECONDARY
 * CONTROLLER_CAPABILITIES_ON_OTHER_NETWORK
 * CONTROLLER_CAPABILITIES_NODEID_SERVER_PRESENT
 * CONTROLLER_CAPABILITIES_IS_REAL_PRIMARY
 * CONTROLLER_CAPABILITIES_IS_SUC
 * CONTROLLER_CAPABILITIES_NO_NODES_INCUDED
 */
int zw_serialapi_get_controller_capability(zw_serialapi_t *sapi, uint8_t *capabilities)
{
   int res;
   zw_serialapi_frame_t *rx_frame;

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES, NULL, 0, &rx_frame);
   if (res == 0)
   {
      *capabilities = rx_frame->data[0];
   }

   osMutexRelease(sapi->api_mutex);

   return res;
}

int zw_serialapi_get_controller_capability_str(zw_serialapi_t *sapi, char *buf, int bufsize)
{
   uint8_t capabilities;

   if (zw_serialapi_get_controller_capability(sapi, &capabilities) != 0)
   {
      return -1;
   }

   if (capabilities & CONTROLLER_CAPABILITIES_NODEID_SERVER_PRESENT)
   {
      if (capabilities & CONTROLLER_CAPABILITIES_IS_SUC)
      {
         snprintf(buf, bufsize, "SIS");
      }
      else
      {
         snprintf(buf, bufsize, "Inclusion");
      }
   }
   else
   {
      if (capabilities & CONTROLLER_CAPABILITIES_IS_SUC)
      {
         snprintf(buf, bufsize, "SUC");
      }
      else
      {
         if (capabilities & CONTROLLER_CAPABILITIES_IS_SECONDARY)
         {
            snprintf(buf, bufsize, "Secondary");
         }
         else
         {
            snprintf(buf, bufsize, "Primary");
         }
      }
   }

   return 0;
}

/** Add node to network */
int zw_serialapi_add_node_to_network(zw_serialapi_t *sapi, uint8_t mode)
{
   int res;
   zw_serialapi_frame_t *rx_frame;
   uint8_t params[2] = {mode, SEQNO(sapi)};

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_ZW_ADD_NODE_TO_NETWORK, params, 2, &rx_frame);

   osMutexRelease(sapi->api_mutex);

   return res;
}

/** Remove node to network */
int zw_serialapi_remove_node_from_network(zw_serialapi_t *sapi, uint8_t mode)
{
   int res;
   zw_serialapi_frame_t *rx_frame;
   uint8_t params[2] = {mode, SEQNO(sapi)};

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK, params, 2, &rx_frame);

   osMutexRelease(sapi->api_mutex);

   return res;
}

/** Remove failed node from network */
int zw_serialapi_remove_failed_node(zw_serialapi_t *sapi, uint8_t node_id)
{
   int res;
   zw_serialapi_frame_t *rx_frame;
   uint8_t params[] = {node_id, SEQNO(sapi)};

   osMutexWait(sapi->api_mutex, osWaitForever);

   // Send request and wait for response
   res = zw_serialapi_send_request(sapi, FUNC_ID_ZW_REMOVE_FAILED_NODE_ID, params, sizeof(params), &rx_frame);

   osMutexRelease(sapi->api_mutex);

   return res;
}


/** Set user code in node */
int zw_serialapi_set_usercode(zw_serialapi_t *sapi, uint8_t node_id, uint8_t uid, const char *user_code)
{
   int idx = 0;
   uint8_t buf[64];

   if (uid == 0 || uid > ZW_MAX_NUM_USERCODES)
   {
      TRACE_ERROR("User code ID out of range");
      return -1;
   }

   buf[idx++] = COMMAND_CLASS_USER_CODE;
   buf[idx++] = USER_CODE_SET;
   buf[idx++] = uid;
   buf[idx++] = 0x00;
   while(*user_code != '\0')
   {
      if (idx == sizeof(buf))
      {
         TRACE_ERROR("Usercode '%s' buffer overflow", user_code);
         return -1;
      }

      buf[idx++] = *user_code++;
   }

   return zw_serialapi_send_data(sapi, node_id, buf, idx);
}

/** Get user code from node */
int zw_serialapi_get_usercode(zw_serialapi_t *sapi, uint8_t node_id, uint8_t uid, char *user_code, int user_code_size)
{
   int res, len, ix;
   uint8_t buf[64];
   uint8_t bufsize = sizeof(buf);

   if (uid == 0 || uid > ZW_MAX_NUM_USERCODES)
   {
      TRACE_ERROR("User code ID out of range");
      return -1;
   }

   buf[0] = COMMAND_CLASS_USER_CODE;
   buf[1] = USER_CODE_GET;
   buf[2] = uid;

   res = zw_serialapi_sendrecv_data(sapi, node_id, buf, 3, buf, &bufsize);
   if (res == 0)
   {
      len = bufsize - 4;
      if (user_code_size < len + 1)
      {
         TRACE_ERROR("Usercode buffer overflow, %d < %d", user_code_size, len+1);
         return -1;
      }

      for (ix = 0; ix < len; ix++)
      {
         user_code[ix] = buf[ix + 4];
      }
      user_code[ix] = '\0';

      TRACE("Received node_id: %d  len: %d  usercode[%d]: '%s' ", node_id, len, uid, user_code);
   }

   return res;
}

int zw_serialapi_get_memory(zw_serialapi_t *sapi, uint16_t addr, uint8_t *buf, int count)
{
   return 0;
}

int zw_serialapi_set_memory(zw_serialapi_t *sapi, uint16_t addr, uint8_t *buf, int count)
{
   return 0;
}



/** Return node type as string */
const char *zw_serialapi_get_node_type_str(uint8_t node_type)
{
   switch (node_type)
   {
      case GENERIC_TYPE_GENERIC_CONTROLLER:
         return "Generic Controller";

      case GENERIC_TYPE_STATIC_CONTROLLER:
         return "Static Controller";

      case GENERIC_TYPE_REPEATER_SLAVE:
         return "Repeater Slave";

      case GENERIC_TYPE_SWITCH_BINARY:
         return "Binary Switch";

      case GENERIC_TYPE_SWITCH_MULTILEVEL:
         return "Multilevel Switch";

      case GENERIC_TYPE_SWITCH_REMOTE:
         return "Remote Switch";

      case GENERIC_TYPE_SWITCH_TOGGLE:
         return "Toggle Switch";

      case GENERIC_TYPE_SENSOR_BINARY:
         return "Binary Sensor";

      case GENERIC_TYPE_SENSOR_MULTILEVEL:
         return "Sensor Multilevel";

      case GENERIC_TYPE_SENSOR_ALARM:
         return "Sensor Alarm";

      case GENERIC_TYPE_METER:
         return "Meter";

      case GENERIC_TYPE_METER_PULSE:
         return "Pulse Meter";

      case GENERIC_TYPE_ENTRY_CONTROL:
         return "Entry Control";

      case GENERIC_TYPE_AV_CONTROL_POINT:
         return "AV Control Point";

      case GENERIC_TYPE_DISPLAY:
         return "Display";

      case GENERIC_TYPE_SEMI_INTEROPERABLE:
         return "Semi Interoperable";

      case GENERIC_TYPE_NON_INTEROPERABLE:
         return "Non Interoperable";

      case GENERIC_TYPE_THERMOSTAT:
         return "Thermostat";

      case GENERIC_TYPE_VENTILATION:
         return "Ventilation";

      case GENERIC_TYPE_WINDOW_COVERING:
         return "Window Covering";

      case GENERIC_TYPE_SECURITY_PANEL:
         return "Security Panel";

      case GENERIC_TYPE_WALL_CONTROLLER:
         return "Wall Controller";

      case GENERIC_TYPE_APPLIANCE:
         return "Appliance";

      case GENERIC_TYPE_SENSOR_NOTIFICATION:
         return "Sensor Notification";

      case GENERIC_TYPE_NETWORK_EXTENDER:
         return "Network Extender";

      case GENERIC_TYPE_ZIP_NODE:
         return "Zip Node";

      case 0:			/* No Nodetype registered */
         return "No device";

      default:
         return "Unknown Device type";
   }
}
