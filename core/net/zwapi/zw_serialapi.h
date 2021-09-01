

#ifndef __ZW_SERIALAPI_H
#define __ZW_SERIALAPI_H

#include "zw_eeprom.h"
#include "zw_serialapi_link.h"
#include "zw_classcmd.h"


/* Flags used in SERIAL_API_GET_INIT_DATA */
#define GET_INIT_DATA_FLAG_SLAVE_API                    0x01
#define GET_INIT_DATA_FLAG_TIMER_SUPPORT                0x02
#define GET_INIT_DATA_FLAG_CONTROLLER_STATUS            0x04 /* Obsolete. USE next */
#define GET_INIT_DATA_FLAG_SECONDARY_CTRL               0x04
#define GET_INIT_DATA_FLAG_IS_SUC                       0x08


/* Function IDs - Definitions */
#define FUNC_ID_SERIAL_API_GET_INIT_DATA                0x02
#define FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION        0x03
#define FUNC_ID_APPLICATION_COMMAND_HANDLER             0x04
#define FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES          0x05

/* SERIAL API ver 4 added - START */
#define FUNC_ID_SERIAL_API_SET_TIMEOUTS                 0x06
#define FUNC_ID_SERIAL_API_GET_CAPABILITIES             0x07
#define FUNC_ID_SERIAL_API_SOFT_RESET                   0x08
/* SERIAL API ver 4 added - END */

#define FUNC_ID_ZW_SET_RF_RECEIVE_MODE                  0x10
#define FUNC_ID_ZW_SET_SLEEP_MODE                       0x11
#define FUNC_ID_ZW_SEND_NODE_INFORMATION                0x12
#define FUNC_ID_ZW_SEND_DATA                            0x13
#define FUNC_ID_ZW_SEND_DATA_MULTI                      0x14
#define FUNC_ID_ZW_GET_VERSION                          0x15

/* SERIAL API ver 4 added - START */
#define FUNC_ID_ZW_SEND_DATA_ABORT                      0x16
#define FUNC_ID_ZW_RF_POWER_LEVEL_SET                   0x17
#define FUNC_ID_ZW_SEND_DATA_META                       0x18
/* SERIAL API ver 4 added - END */

#define FUNC_ID_ZW_SEND_DATA_MR                         0x19
#define FUNC_ID_ZW_SEND_DATA_META_MR                    0x1A
#define FUNC_ID_ZW_SET_ROUTING_INFO                     0x1B

#define FUNC_ID_ZW_GET_RANDOM                           0x1C
#define FUNC_ID_ZW_RANDOM                               0x1D
#define FUNC_ID_ZW_RF_POWER_LEVEL_REDISCOVERY_SET       0x1E
/* ZW030x only */
#define FUNC_ID_APPLICATION_RF_NOTIFY                   0x1F
/* ZW030x only end */

#define FUNC_ID_MEMORY_GET_ID                           0x20
#define FUNC_ID_MEMORY_GET_BYTE                         0x21
#define FUNC_ID_MEMORY_PUT_BYTE                         0x22
#define FUNC_ID_MEMORY_GET_BUFFER                       0x23
#define FUNC_ID_MEMORY_PUT_BUFFER                       0x24
/* SERIAL API ver 5 added - START */
#define FUNC_ID_SERIAL_API_GET_APPL_HOST_MEMORY_OFFSET  0x25
#define FUNC_ID_DEBUG_OUTPUT                            0x26
/* SERIAL API ver 5 added - END */

#define FUNC_ID_CLOCK_SET                               0x30
#define FUNC_ID_CLOCK_GET                               0x31
#define FUNC_ID_CLOCK_CMP                               0x32
#define FUNC_ID_RTC_TIMER_CREATE                        0x33
#define FUNC_ID_RTC_TIMER_READ                          0x34
#define FUNC_ID_RTC_TIMER_DELETE                        0x35
#define FUNC_ID_RTC_TIMER_CALL                          0x36

#define FUNC_ID_CLEAR_TX_TIMERS                         0x37
#define FUNC_ID_GET_TX_TIMERS                           0x38

#define FUNC_ID_ZW_SET_LEARN_NODE_STATE                 0x40
#define FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO               0x41
#define FUNC_ID_ZW_SET_DEFAULT                          0x42
#define FUNC_ID_ZW_NEW_CONTROLLER                       0x43
#define FUNC_ID_ZW_REPLICATION_COMMAND_COMPLETE         0x44
#define FUNC_ID_ZW_REPLICATION_SEND_DATA                0x45
#define FUNC_ID_ZW_ASSIGN_RETURN_ROUTE                  0x46
#define FUNC_ID_ZW_DELETE_RETURN_ROUTE                  0x47
#define FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE         0x48
#define FUNC_ID_ZW_APPLICATION_UPDATE                   0x49

/*Obsolete use ZW_APPLICATION_UPDATE */
#define FUNC_ID_ZW_APPLICATION_CONTROLLER_UPDATE        0x49

#define FUNC_ID_ZW_ADD_NODE_TO_NETWORK                  0x4A
#define FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK             0x4B
#define FUNC_ID_ZW_CREATE_NEW_PRIMARY                   0x4C
#define FUNC_ID_ZW_CONTROLLER_CHANGE                    0x4D

#define FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE_MR      0x4E
#define FUNC_ID_ZW_ASSIGN_RETURN_ROUTE_MR               0x4F

/* Slave only */
#define FUNC_ID_ZW_SET_LEARN_MODE                       0x50
/* Slave only end */

#define FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE              0x51
#define FUNC_ID_ZW_ENABLE_SUC                           0x52
#define FUNC_ID_ZW_REQUEST_NETWORK_UPDATE               0x53
#define FUNC_ID_ZW_SET_SUC_NODE_ID                      0x54
#define FUNC_ID_ZW_DELETE_SUC_RETURN_ROUTE              0x55
#define FUNC_ID_ZW_GET_SUC_NODE_ID                      0x56
#define FUNC_ID_ZW_SEND_SUC_ID                          0x57

#define FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE_MR           0x58
#define FUNC_ID_ZW_REDISCOVERY_NEEDED                   0x59

#define FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE_OPTION	0x5A

/* Slave only */
#define FUNC_ID_ZW_SUPPORT9600_ONLY                     0x5B
/* Slave only end */

/* Enhanced/Routing Slave only */
#define FUNC_ID_ZW_REQUEST_NEW_ROUTE_DESTINATIONS       0x5C
#define FUNC_ID_ZW_IS_NODE_WITHIN_DIRECT_RANGE          0x5D
/* Enhanced/Routing Slave only end */

#define FUNC_ID_ZW_EXPLORE_REQUEST_INCLUSION            0x5E

#define FUNC_ID_ZW_REQUEST_NODE_INFO                    0x60
#define FUNC_ID_ZW_REMOVE_FAILED_NODE_ID                0x61
#define FUNC_ID_ZW_IS_FAILED_NODE_ID                    0x62
#define FUNC_ID_ZW_REPLACE_FAILED_NODE                  0x63

#define FUNC_ID_TIMER_START                             0x70
#define FUNC_ID_TIMER_RESTART                           0x71
#define FUNC_ID_TIMER_CANCEL                            0x72
#define FUNC_ID_TIMER_CALL                              0x73

/* Installer API */
#define FUNC_ID_GET_ROUTING_TABLE_LINE                  0x80
#define FUNC_ID_GET_TX_COUNTER                          0x81
#define FUNC_ID_RESET_TX_COUNTER                        0x82
#define FUNC_ID_STORE_NODEINFO                          0x83
#define FUNC_ID_STORE_HOMEID                            0x84
/* Installer API only end */

#define FUNC_ID_LOCK_ROUTE_RESPONSE                     0x90

#define FUNC_ID_ZW_GET_LAST_WORKING_ROUTE               0x92
#define FUNC_ID_ZW_SET_LAST_WORKING_ROUTE               0x93

#ifdef ZW_CONTROLLER_SINGLE
#define FUNC_ID_SERIAL_API_TEST                         0x95
#endif

/* ZW_CONTROLLER_BRIDGE only START */
#define FUNC_ID_SERIAL_API_APPL_SLAVE_NODE_INFORMATION  0xA0
#define FUNC_ID_APPLICATION_SLAVE_COMMAND_HANDLER       0xA1
#define FUNC_ID_ZW_SEND_SLAVE_NODE_INFORMATION          0xA2
#define FUNC_ID_ZW_SEND_SLAVE_DATA                      0xA3
#define FUNC_ID_ZW_SET_SLAVE_LEARN_MODE                 0xA4
#define FUNC_ID_ZW_GET_VIRTUAL_NODES                    0xA5
#define FUNC_ID_ZW_IS_VIRTUAL_NODE                      0xA6
#define FUNC_ID_ZW_SEND_SLAVE_DATA_MR                   0xA7
/* DevKit 4.5x added - obsoletes FUNC_ID_APPLICATION_SLAVE_COMMAND_HANDLER and */
/* FUNC_ID_APPLICATION_COMMAND_HANDLER for the Controller Bridge applications as */
/* this handles both cases - only for 4.5x based Controller Bridge applications */
#define FUNC_ID_APPLICATION_COMMAND_HANDLER_BRIDGE      0xA8
/* DevKit 4.5x added - Adds sourceNodeID to the parameter list */
#define FUNC_ID_ZW_SEND_DATA_BRIDGE                     0xA9
#define FUNC_ID_ZW_SEND_DATA_META_BRIDGE                0xAA
#define FUNC_ID_ZW_SEND_DATA_MULTI_BRIDGE               0xAB
/* ZW_CONTROLLER_BRIDGE only END */

#define FUNC_ID_PWR_SETSTOPMODE                         0xB0    // ZW102 only
#define FUNC_ID_PWR_CLK_PD                              0xB1    // ZW102 only
#define FUNC_ID_PWR_CLK_PUP                             0xB2    // ZW102 only
#define FUNC_ID_PWR_SELECT_CLK                          0xB3    // ZW102 only
#define FUNC_ID_ZW_SET_WUT_TIMEOUT                      0xB4    // ZW201 only
#define FUNC_ID_ZW_IS_WUT_KICKED                        0xB5    // ZW201 only

#define FUNC_ID_ZW_WATCHDOG_ENABLE                      0xB6
#define FUNC_ID_ZW_WATCHDOG_DISABLE                     0xB7
#define FUNC_ID_ZW_WATCHDOG_KICK                        0xB8
#define FUNC_ID_ZW_SET_EXT_INT_LEVEL                    0xB9    // ZW201 only

#define FUNC_ID_ZW_RF_POWER_LEVEL_GET                   0xBA
#define FUNC_ID_ZW_GET_NEIGHBOR_COUNT                   0xBB
#define FUNC_ID_ZW_ARE_NODES_NEIGHBOURS                 0xBC

#define FUNC_ID_ZW_TYPE_LIBRARY                         0xBD
#define FUNC_ID_ZW_SEND_TEST_FRAME                      0xBE
#define FUNC_ID_ZW_GET_PROTOCOL_STATUS                  0xBF

#define FUNC_ID_ZW_SET_PROMISCUOUS_MODE                 0xD0
/* SERIAL API ver 5 added - START */
#define FUNC_ID_PROMISCUOUS_APPLICATION_COMMAND_HANDLER 0xD1
/* SERIAL API ver 5 added - END */

#define FUNC_ID_ZW_WATCHDOG_START                       0xD2
#define FUNC_ID_ZW_WATCHDOG_STOP                        0xD3

#define FUNC_ID_ZW_SET_ROUTING_MAX                      0xD4
#define OBSOLETE_FUNC_ID_ZW_GET_ROUTING_MAX             0xD5

/* Allocated for Power Management */
#define FUNC_ID_SERIAL_API_POWER_MANAGEMENT				0xEE
#define FUNC_ID_SERIAL_API_READY						0xEF

/* Allocated for proprietary serial API commands */
#define FUNC_ID_PROPRIETARY_0                           0xF0
#define FUNC_ID_PROPRIETARY_1                           0xF1
#define FUNC_ID_PROPRIETARY_2                           0xF2
#define FUNC_ID_PROPRIETARY_3                           0xF3
#define FUNC_ID_PROPRIETARY_4                           0xF4
#define FUNC_ID_PROPRIETARY_5                           0xF5
#define FUNC_ID_PROPRIETARY_6                           0xF6
#define FUNC_ID_PROPRIETARY_7                           0xF7
#define FUNC_ID_PROPRIETARY_8                           0xF8
#define FUNC_ID_PROPRIETARY_9                           0xF9
#define FUNC_ID_PROPRIETARY_A                           0xFA
#define FUNC_ID_PROPRIETARY_B                           0xFB
#define FUNC_ID_PROPRIETARY_C                           0xFC
#define FUNC_ID_PROPRIETARY_D                           0xFD
#define FUNC_ID_PROPRIETARY_E                           0xFE


/* Illegal function ID */
#define FUNC_ID_UNKNOWN                                 0xFF


/* Defines for ZW_GetControllerCapabilities */
#define CONTROLLER_CAPABILITIES_IS_SECONDARY                 0x01
#define CONTROLLER_CAPABILITIES_ON_OTHER_NETWORK             0x02
#define CONTROLLER_CAPABILITIES_NODEID_SERVER_PRESENT        0x04
#define CONTROLLER_CAPABILITIES_IS_REAL_PRIMARY              0x08
#define CONTROLLER_CAPABILITIES_IS_SUC                       0x10
#define CONTROLLER_CAPABILITIES_NO_NODES_INCUDED             0x20


// Listening bit in NODEINFO capability
#define NODEINFO_LISTENING_MASK     0x80
#define NODEINFO_LISTENING_SUPPORT  0x80
// Routing bit in the NODEINFO capability byte
#define NODEINFO_ROUTING_SUPPORT            0x40

// Beam wakeup mode type bits in the NODEINFO security byte
#define NODEINFO_ZWAVE_SENSOR_MODE_WAKEUP_1000   0x40
#define NODEINFO_ZWAVE_SENSOR_MODE_WAKEUP_250    0x20


/* Transmit frame option flags */
#define TRANSMIT_OPTION_ACK         0x01   /* request acknowledge from destination node */
#define TRANSMIT_OPTION_LOW_POWER   0x02   /* transmit at low output power level (1/3 of normal RF range)*/
#define TRANSMIT_OPTION_AUTO_ROUTE  0x04   /* request retransmission via repeater nodes */
/* do not use response route - Even if available */
#define TRANSMIT_OPTION_NO_ROUTE      0x10

/* Use explore frame if needed */
#define TRANSMIT_OPTION_EXPLORE       0x20

/* Transmit frame option flag which are valid when sending explore frames  */
#define TRANSMIT_EXPLORE_OPTION_ACK         TRANSMIT_OPTION_ACK
#define TRANSMIT_EXPLORE_OPTION_LOW_POWER   TRANSMIT_OPTION_LOW_POWER


/* Transmit complete codes */
#define TRANSMIT_COMPLETE_OK      0x00
#define TRANSMIT_COMPLETE_NO_ACK  0x01  /* retransmission error */
#define TRANSMIT_COMPLETE_FAIL    0x02  /* transmit error */
/*#ifdef ZW_CONTROLLER*/
/* Assign route transmit complete but no routes was found */
#define TRANSMIT_COMPLETE_NOROUTE 0x04  /* no route found in assignroute */
                                        /* therefore nothing was transmitted */
/*#endif*/
#define TRANSMIT_COMPLETE_HOP_0_FAIL  0x05
#define TRANSMIT_COMPLETE_HOP_1_FAIL  0x06
#define TRANSMIT_COMPLETE_HOP_2_FAIL  0x07
#define TRANSMIT_COMPLETE_HOP_3_FAIL  0x08
#define TRANSMIT_COMPLETE_HOP_4_FAIL  0x09

/* Max hops in route */
#define TRANSMIT_ROUTED_ATTEMPT        0x08

#define ZW_MAX_NUM_NODES               128
#define ZW_RX_TIMEOUT                  2000
#define ZW_TX_OPTIONS                  TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE | TRANSMIT_OPTION_EXPLORE

#define ZW_MAX_NUM_USERCODES          232
#define ZW_USERCODE_MAXLEN            20

#define ZW_CONTROLLER_NODE_ID         0x1


typedef struct
{
   uint8_t basic;
   uint8_t generic;
   uint8_t specific;

} zw_node_type_t;

/**
 * Node info stored within the non-volatile memory
 * This are the first (protocol part) payload bytes from the Node Infomation frame
 */
typedef struct
{
   uint8_t capability;      /* Network capabilities */
   uint8_t security;        /* Network security */
   uint8_t reserved;
   zw_node_type_t node_type;   /* Basic, Generic and Specific Device Types */

} zw_node_info_t;

/** Command frame data */
typedef struct
{
   uint8_t type;           // Type of frame, always == 0
   uint8_t node_addr;      // Node address
   uint8_t length;         // Length of cmd_class + cmd + params
   uint8_t cmd_class;      // Command class
   uint8_t cmd;            // Command
   uint8_t params[];       // Command parameters

} __attribute__ ((packed)) zw_serialapi_cmd_frame_data_t;
#define ZW_CMD_FRAME_PARAMS_LENGTH(_f) ((_f)->length - 2)

#define ZW_CMD_FRAME_DATA_MIN_LENGTH      3


/** Incomming commands handler function callback */
typedef void (*zw_serialapi_command_handler_t)(uint8_t node_addr, uint8_t cmd_class, uint8_t cmd, uint8_t *params, int params_length, void *user_param);

/**
 * ZW serial API object
 */
typedef struct
{
   /** Thread ID */
   osThreadId thread_id;

   /** API mutex */
   osMutexId api_mutex;

   /** ZW serial link */
   zw_serialapi_link_t zwlink;

   /** Last SEQ number for requests to other nodes than controller */
   uint8_t seqno;

   zw_serialapi_frame_t tx_frame;

   /** Receive cmd queue */
   osMessageQId rx_cmd_queue_id;

   /** Response */
   struct
   {
      osMessageQId queue_id;
      uint8_t seqno;             // Expected SEQ number
      uint8_t cmd;               // Expected request cmd as response

   } response;

   /** Options */
   struct
   {
      uint16_t rx_timeout;
      uint8_t tx_options;

   } options;

} zw_serialapi_t;

/** Open serial API */
int zw_serialapi_init(zw_serialapi_t *sapi);

/** Close serial API */
int zw_serialapi_deinit(zw_serialapi_t *sapi);

/** Recv request frame (incomming commands) */
int zw_serialapi_recv_request(zw_serialapi_t *sapi, zw_serialapi_frame_t **frame, uint32_t timeout);

/** Transmit data buffer to a single ZW-node or all ZW-nodes (broadcast). */
int zw_serialapi_send_data_ext(zw_serialapi_t *sapi, uint8_t node_id, uint8_t *data, uint8_t datalen, uint8_t tx_options, uint16_t timeout);
#define zw_serialapi_send_data(_sapi, _node_id, _data, _datalen) \
   zw_serialapi_send_data_ext(_sapi, _node_id, _data, _datalen, (_sapi)->options.tx_options, (_sapi)->options.rx_timeout)

/** Send data and wait for application response data */
int zw_serialapi_sendrecv_data_ext(zw_serialapi_t *sapi, uint8_t node_id, uint8_t *out_data, uint8_t out_datalen,
                                    uint8_t tx_options, uint16_t timeout, uint8_t *in_data, uint8_t *in_datalen);
#define zw_serialapi_sendrecv_data(_sapi, _node_id, _out_data, _out_datalen, _in_data, _in_datalen) \
   zw_serialapi_sendrecv_data_ext(_sapi, _node_id, _out_data, _out_datalen, (_sapi)->options.tx_options, (_sapi)->options.rx_timeout, _in_data, _in_datalen)

/** Test node connection - ping */
int zw_serialapi_ping_node(zw_serialapi_t *sapi, uint8_t node_id, uint16_t timeout);

/**
 * Get the Z-Wave library basis version
 * @param pBuf			OUT Pointer to buffer where version string will be copied to. Buffer must be at least 14 bytes long.
 */
int zw_serialapi_get_version(zw_serialapi_t *sapi, uint8_t *outbuf, uint8_t outbuf_size);

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
int zw_serialapi_get_init_data(zw_serialapi_t *sapi, uint8_t *ver, uint8_t *capabilities, uint8_t *nodes_list, uint8_t *nodes_list_size);

/**
 * Copy the Node's current protocol information from the non-volatile memory.
 */
int zw_serialapi_get_node_protocol_info(zw_serialapi_t *sapi, uint8_t node_id, zw_node_info_t *node_info);

/**
 * Copy the Home-ID and Node-ID to the specified RAM addresses
 */
int zw_serialapi_memory_get_id(zw_serialapi_t *sapi, uint32_t *home_id, uint8_t *node_id);

/** Set HOME-ID and node-ID */
int zw_serialapi_memory_set_id(zw_serialapi_t *sapi, uint32_t home_id, uint8_t node_id);


/** Get the currently registered SUC node ID */
int zw_serialapi_get_SUC_node_id(zw_serialapi_t *sapi, uint8_t *node_id);

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
int zw_serialapi_get_controller_capability(zw_serialapi_t *sapi, uint8_t *capabilities);
int zw_serialapi_get_controller_capability_str(zw_serialapi_t *sapi, char *buf, int bufsize);

/** Add node to network */
int zw_serialapi_add_node_to_network(zw_serialapi_t *sapi, uint8_t mode);

/** Remove node to network */
int zw_serialapi_remove_node_from_network(zw_serialapi_t *sapi, uint8_t mode);

/** Remove failed node from network */
int zw_serialapi_remove_failed_node(zw_serialapi_t *sapi, uint8_t node_id);

/** Set user code in node */
int zw_serialapi_set_usercode(zw_serialapi_t *sapi, uint8_t node_id, uint8_t uid, const char *user_code);

/** Get user code from node */
int zw_serialapi_get_usercode(zw_serialapi_t *sapi, uint8_t node_id, uint8_t uid, char *user_code, int user_code_size);

/** Return node type as string */
const char *zw_serialapi_get_node_type_str(uint8_t node_type);

int zw_serialapi_get_memory(zw_serialapi_t *sapi, uint16_t addr, uint8_t *buf, int count);
int zw_serialapi_set_memory(zw_serialapi_t *sapi, uint16_t addr, uint8_t *buf, int count);


#endif  // __ZW_SERIALAPI_H
