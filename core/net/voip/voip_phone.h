
#ifndef __voip_phone_h
#define __voip_phone_h

#include "voip.h"
#include "voip_audio.h"
#include "voip_video.h"

//----------------------------------------------------------------------
//                         VOIP phone configuration
//----------------------------------------------------------------------

#define USE_G711_CODEC                      1
#define USE_C2_CODEC                        0

#define VOIP_AUDIO_UDP_PORT                 9000            //! RTP base audio port
#define VOIP_VIDEO_UDP_PORT                 16200           //! RTP base video port

#if USE_G711_CODEC
 #define VOIP_AUDIO_CODEC_TYPE               RTP_PCMA_PAYLOADTYPE
 #define VOIP_AUDIO_CODEC_NAME               RTP_PCMA_CODEC_NAME
 #define VOIP_AUDIO_CODEC_CLOCK_RATE         RTP_PCMA_CLOCK_RATE
 #define VOIP_AUDIO_CODEC_SAMPLES_PER_FRAME  RTP_PCMA_SAMPLES_PER_FRAME
#elif USE_C2_CODEC
 #define VOIP_AUDIO_CODEC_TYPE               RTP_CODEC2_PAYLOADTYPE
 #define VOIP_AUDIO_CODEC_NAME               RTP_CODEC2_CODEC_NAME
 #define VOIP_AUDIO_CODEC_CLOCK_RATE         RTP_CODEC2_CLOCK_RATE
 #define VOIP_AUDIO_CODEC_SAMPLES_PER_FRAME  RTP_CODEC2_SAMPLES_PER_FRAME
#else
 #error Not specified VOIP audio codec
#endif

#define VOIP_PHONE_CALL_NUMBER_LEN      20


/** VOIP phone configuration */
typedef struct
{
    /** SIP profile cfg */
    sip_profile_t sip_profile;

    /** SDP media cfg */
    sip_sdp_media_t sdp_media;

    /** Call number for auto start */
    char call_number[VOIP_PHONE_CALL_NUMBER_LEN];

} voip_phone_config_t;


/** VOIP phone states */
typedef enum
{
    VOIP_PHONE_STATE_IDLE,
    VOIP_PHONE_STATE_WAITFOR_REGISTER,
    VOIP_PHONE_STATE_READY,
    VOIP_PHONE_STATE_CONNECTING_OUT,
    VOIP_PHONE_STATE_CONNECTING_IN,
    VOIP_PHONE_STATE_CONNECTED,
    VOIP_PHONE_STATE_HANGUP_PENDING,
    VOIP_PHONE_STATE_HANGUP_AND_INVITE,
    VOIP_PHONE_STATE_ERROR,

} voip_phone_state_e;

struct voip_phone;

/**
 * SIP connections events
 */
typedef struct voip_phone_events
{
    /** calling event when connection success registered */
    void (*on_register)(struct voip_phone *phone, const sip_profile_t *prof);

    /** calling when connection success unregistered */
    void (*on_unregister)(struct voip_phone *phone, const sip_profile_t *prof);

    /** calling when is success invite */
    void (*on_invite)(struct voip_phone *phone, const char *remoteUri, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media);

    /** on invite trying */
    void (*on_invite_trying)(struct voip_phone *phone, const char *remoteUri);

    /** on invite ringing */
    void (*on_invite_ringing)(struct voip_phone *phone, const char *remoteUri);

    /** calling when is connection hangup */
    void (*on_hangup)(struct voip_phone *phone, const char *remoteUri);

    /** Invoked on decline outgoing invite request */
    void (*on_invite_declined)(struct voip_phone *phone, const char *remoteUri);

    /** calling when incomming invite call is comming */
    void (*on_incomming_invite_ringing)(struct voip_phone *phone, const char *remoteUri);

    /** calling on some connection error */
    void (*on_error)(struct voip_phone *phone, uint16_t sipErrCode, sip_err_t errcode);

} voip_phone_events_t;


typedef struct voip_phone
{
    /** Phone events handler */
    const voip_phone_events_t *events;

    /** Phone configuration */
    const voip_phone_config_t *cfg;

    /** Phone state */
    voip_phone_state_e state;

    /** SIP connection */
    sip_con_t sipcon;

    /** Outgoing call number */
    char call_number[VOIP_PHONE_CALL_NUMBER_LEN];

    /** Invoke after register */
    bool call_after_register;

    /** Phone audio */
    voip_audio_t audio;

    /** Video */
#if defined(CFG_VOIP_VIDEO_ENABLE) && (CFG_VOIP_VIDEO_ENABLE == 1)
    voip_video_t video;
#endif

} voip_phone_t;


/** Initialize VOIP phone */
int voip_phone_init(voip_phone_t *voip, const voip_phone_events_t *events);

/** Deinitialize VOIP phone */
int voip_phone_deinit(voip_phone_t *voip);

/** Start VOIP phone */
int voip_phone_start(voip_phone_t *voip, const voip_phone_config_t *cfg);

/** Start VOIP phone with call */
int voip_phone_start_call(voip_phone_t *voip, const voip_phone_config_t *cfg, const char *call_number);

/** Stop VOIP phone */
int voip_phone_stop(voip_phone_t *voip);

/** Outgoing call */
int voip_phone_call(voip_phone_t *voip, const char *call_number);

/** Hangup call */
int voip_phone_hangup(voip_phone_t *voip);

#endif // __voip_phone_h
