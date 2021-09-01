/**
 * \file rtp.h				\brief RTP protocol implementation
 */

#ifndef __rtp_h
#define __rtp_h

#include "pjmedia/rtp.h"
#include "pjmedia/rtcp.h"

#include "voip_platform.h"

#ifndef DEBUG_RTP
#define DEBUG_RTP       0
#endif

#define RTCP_PORT(rtp_port)            (rtp_port+1)
#define RTPC_SEND_PACKET_INTERVAL      30000        //! interval to send RTCP packet

#define RTP_HEADER_SIZE                sizeof(pjmedia_rtp_hdr)
#define RTP_DTMF_EVENT_SIZE            sizeof(rtp_dtmf_event_t)

#define RTP_SEND_DTMF_END_COUNT        3
#define RTP_SEND_DTMF_STEP             80
#define RTP_SEND_DTMF_VOLUME           10

//
// Payload types
//
#define RTP_PCMA_PAYLOADTYPE           8           //! PCMA payload type
#define RTP_G722_PAYLOADTYPE           9           //! G722 payload type (clock rate 8000Hz)
#define RTP_L16_2_PAYLOADTYPE          10          //! 16bit stereo PCM
#define RTP_L16_PAYLOADTYPE            11          //! 16bit mono PCM
#define RTP_CODEC2_PAYLOADTYPE         2           //! C2 codec payload type
#define RTP_EVENT_PAYLOADTYPE          101         //! RTP event payload type
#define RTP_EVENT2_PAYLOADTYPE         104         //! RTP event2 payload type
#define RTP_EVENT3_PAYLOADTYPE         96          //! RTP event3 payload type
#define RTP_COMFORT_NOISE_PAYLOADTYPE  13          //! Generate comfort noise payload type

#define DEFAULT_RTP_EVENT_PAYLOADTYPE  RTP_EVENT3_PAYLOADTYPE

//
// Codec params
//
#define RTP_PCMA_CODEC_NAME             "pcma"
#define RTP_PCMA_SAMPLES_PER_FRAME      160             //! G711 samples per frame
#define RTP_PCMA_CLOCK_RATE             8000            //! sampling freq

#define RTP_CODEC2_CODEC_NAME           "x-codec2"
#define RTP_CODEC2_SAMPLES_PER_FRAME    160             //! Codec2 samples per frame
#define RTP_CODEC2_CLOCK_RATE           8000            //! sampling freq


// forward decl.
struct rtp_connection_events_t;

/** DTMF RTP event */
typedef struct
{
#if PJ_IS_BIG_ENDIAN
    uint8_t id;
    uint8_t e:1;
    uint8_t r:1;
    uint8_t volume:6;
    uint16_t duration;
#else
    uint8_t id;
    uint8_t volume:6;
    uint8_t r:1;
    uint8_t e:1;
    uint16_t duration;
#endif
} __attribute__ ((packed)) rtp_dtmf_event_t;


/** DTMF codes */
typedef enum
{
    RTP_DTMF_CODE_0 = 0,
    RTP_DTMF_CODE_1,
    RTP_DTMF_CODE_2,
    RTP_DTMF_CODE_3,
    RTP_DTMF_CODE_4,
    RTP_DTMF_CODE_5,
    RTP_DTMF_CODE_6,
    RTP_DTMF_CODE_7,
    RTP_DTMF_CODE_8,
    RTP_DTMF_CODE_9,
    RTP_DTMF_CODE_ASTERISK,
    RTP_DTMF_CODE_HASH,
    RTP_DTMF_CODE_A,
    RTP_DTMF_CODE_B,
    RTP_DTMF_CODE_C,
    RTP_DTMF_CODE_D,
    RTP_DTMF_CODE_UNKNOWN

} rtp_dtmf_code_t;

/** Send DTMF states */
typedef enum
{

    RTP_SEND_DTMF_EVENT_STATE_NONE,
    RTP_SEND_DTMF_EVENT_STATE_PENDING_MARK,
    RTP_SEND_DTMF_EVENT_STATE_PENDING,
    RTP_SEND_DTMF_EVENT_STATE_END

} rtp_send_dmtf_event_state_t;


/** RTP connection */
typedef struct
{
    uint32_t ssrc;
    uint16_t payloadType;         //! RTP payload type
    uint16_t bytesPerFrame;       //! number of bytes per frame
    uint16_t clockRate;           //! rtp clock rate
    uint8_t telephone_event;      //! DTMF telephone event

    const struct rtp_connection_events_t *events;

    /** Open socket flag */
    bool socket_is_open;

    /** RTP UDP connection */
    voip_udp_socket_t socket;

    voip_ipaddr_t localAddr;
    uint16_t localPort;

    voip_ipaddr_t remoteAddr;
    uint16_t remotePort;

    /** RTP audio session */
    pjmedia_rtp_session rtpSes;

    /** RTP DTMF session*/
    pjmedia_rtp_session rtpSes_dtmf;

    /** RTPC session */
    pjmedia_rtcp_session rtpcSes;
    int rtpc_counter;

    /** send audio data buffer */
    uint8_t *sndbuf_audio;

    /** send dtmf buffer */
    uint8_t *sndbuf_dtmf;

    /** current receive dmft event */
    rtp_dtmf_event_t dtmf_event;

    /** number of received rtp packets */
    int numReceivedPackets;

    /** Send DMTF event state */
    struct
    {
        rtp_send_dmtf_event_state_t state;
        int duration;
        int cnt;
        rtp_dtmf_code_t code;

    } send_dtmf_event;

    /** pointer to user data */
    void *userData;

} rtp_connection_t;


/** RTP connection event listener */
typedef struct rtp_connection_events_t
{
    /** calling event when connection success registered */
    uint16_t (*on_send_frame_data)(rtp_connection_t *rcon, uint8_t *buf, uint16_t buflen, void *user_param);

    /** function is invoked when are received frame data */
    void (*on_receive_frame_data)(rtp_connection_t *con, const pjmedia_rtp_hdr *hdr, uint8_t *buf, uint16_t count, void *user_param);

    /** function is invoked when is received all rtp packet (rtp header + data) */
    void (*on_receive_frame)(rtp_connection_t *con, uint8_t *buf, int buflen, void *user_param);

    /** function is invoked when is received dtmf event */
    void (*on_receive_dtmf)(rtp_connection_t *con, rtp_dtmf_code_t code, uint16_t duration, uint16_t volume, void *user_param);

    /** Callback invoked after DTMF char sent */
    void (*on_sent_dtmf)(rtp_connection_t *con, rtp_dtmf_code_t code, void *user_param);

    /** Callback invoked when comfort noise packet received */
    void (*on_receive_comfort_noise)(rtp_connection_t *con, void *user_param);

} rtp_connection_events_t;


/** RTP application process call */
void rtp_task(rtp_connection_t *s);

/** Create new RTP connection */
int rtp_create_connection(
            rtp_connection_t *rc,
            voip_ipaddr_t *remoteAddr,
            uint16_t remotePort,
            uint16_t localPort,
            uint16_t payloadType,
            uint16_t bytesPerFrame,
            uint16_t clockRate,
            uint8_t telephone_event,
            uint32_t ssrc,
            const rtp_connection_events_t *events,
            void *userData);

/** Destroy RTP connection */
void rtp_destroy_connection(rtp_connection_t *rc);

/** Send DMTF event */
void rtp_send_dtmf(rtp_connection_t *rc, rtp_dtmf_code_t dtmf_code, uint16_t duration);

#define rtp_dtmf_send_pending(_rc) ((_rc)->send_dtmf_event.state != RTP_SEND_DTMF_EVENT_STATE_NONE)

/** Return true when is RTP connection active */
#define rtp_is_active(rc) \
   ((*rc).socket_is_open)

/** Convert dtmf code to ascii */
char rtp_dtmfcode_to_ascii(rtp_dtmf_code_t code);

/** Convert ascii to code */
rtp_dtmf_code_t rtp_ascii_to_dtmfcode(char c);

/** Set session SSRC */
#define rtp_set_ssrc(rc, sender_ssrc) \
   (rc)->rtpSes.out_hdr.ssrc = pj_htonl(sender_ssrc)

#endif		// rtp.h
