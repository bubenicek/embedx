
#ifndef __voip_config_h
#define __voip_config_h

//
// Debug traces
//

#define ENABLE_TRACE_VOIP           1
#define ENABLE_TRACE_SIP            1
#define ENABLE_TRACE_SIP_RXTX       0
#define ENABLE_TRACE_SIP_AUTH       0
#define ENABLE_TRACE_SIP_INVITE     0
#define ENABLE_TRACE_SIP_REGISTER   0
#define ENABLE_TRACE_RTP            0
#define ENABLE_TRACE_VOIP_AUDIO     1
#define ENABLE_TRACE_AUDIOCODEC     1
#define ENABLE_TRACE_VOIP_VIDEO     1


//
// VOIP platforms
//

#ifndef VOIP_PLATFORM_WICED
#define VOIP_PLATFORM_WICED    0
#endif

#ifndef VOIP_PLATFORM_LWIP
#define VOIP_PLATFORM_LWIP     0
#endif

//
// SIP configuration
//

/** SIP listening port */
#ifndef SIP_PORT
#define SIP_PORT              5060
#endif

/** SIP receive response timeout in ms */
#ifndef SIP_RCV_TMO
#define SIP_RCV_TMO           500
#endif

/** try count send SIP message */
#ifndef SIP_SEND_TRYCNT
#define SIP_SEND_TRYCNT       5
#endif

/** common SIP buffer size */
#ifndef SIP_BUFFER_SIZE
#define SIP_BUFFER_SIZE       1500
#endif

/** SIP UDP connection poll interval */
#ifndef SIP_POLL_INTERVAL
#define SIP_POLL_INTERVAL     100
#endif

#ifndef CFG_VOIP_VIDEO_ENABLE
#define CFG_VOIP_VIDEO_ENABLE                1
#endif

#ifndef CFG_VOIP_AUDIO_DRIVER_NAME
#define CFG_VOIP_AUDIO_DRIVER_NAME           "max9860"
#endif

#ifndef CFG_VOIP_VIDEO_DRIVER_NAME
//#define CFG_VOIP_VIDEO_DRIVER_NAME           "test-jpeg"
#define CFG_VOIP_VIDEO_DRIVER_NAME           "camera"
#endif


#endif // __voip_config_h
