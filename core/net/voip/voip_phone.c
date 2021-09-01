
#include "system.h"
#include "voip_phone.h"

#define TRACE_TAG "voip-phone"

// Prototypes:
static void on_register(sip_con_t *sc, const sip_profile_t *prof);
static void on_unregister(sip_con_t *sc, const sip_profile_t *prof);
static void on_error(sip_con_t *sc, uint16_t sipErrCode, sip_err_t errcode);
static void on_invite(sip_con_t *sc, const char *remoteUri, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media);
static void on_invite_trying(sip_con_t *sc, const char *remoteUri);
static void on_invite_ringing(sip_con_t *sc, const char *remoteUri);
static void on_invite_declined(sip_con_t *sc, const char *remoteUri);
static void on_hangup(sip_con_t *sc, const char *remoteUri);
static void on_incomming_invite_ringing(sip_con_t *sc, const char *remoteUri);


/** SIP callbacks events */
static const sip_con_events_t sip_events =
{
    .on_register = on_register,
    .on_unregister = on_unregister,
    .on_invite = on_invite,
    .on_invite_trying = on_invite_trying,
    .on_invite_ringing = on_invite_ringing,
    .on_invite_declined = on_invite_declined,
    .on_incomming_invite_ringing = on_incomming_invite_ringing,
    .on_hangup = on_hangup,
    .on_error = on_error,
};


/** Initialize VOIP phone */
int voip_phone_init(voip_phone_t *voip, const voip_phone_events_t *events)
{
    memset(voip, 0, sizeof(voip_phone_t));
    voip->events = events;

    if (audio_driver_init() != 0)
    {
        TRACE_ERROR("Initializer audio driver failed");
        return -1;
    }

    return 0;
}

/** Deinitialize VOIP phone */
int voip_phone_deinit(voip_phone_t *voip)
{
    return 0;
}

/** Start VOIP phone */
int voip_phone_start(voip_phone_t *voip, const voip_phone_config_t *cfg)
{
    voip->cfg = cfg;

    if (sip_create_con(
                &voip->sipcon,
                voip->cfg->sip_profile.srv_addr,
                SIP_PORT,            // remote port
                SIP_PORT,            // local port
                &voip->cfg->sdp_media,
                &sip_events,
                voip) != 0)
    {
        TRACE_ERROR("create SIP connection");
        goto fail_start;
    }

    if (sip_register(&voip->sipcon, &voip->cfg->sip_profile, 0) != 0)
    {
        TRACE_ERROR("sip register");
        goto fail_start;
    }

    voip->state = VOIP_PHONE_STATE_WAITFOR_REGISTER;
    TRACE("SIP phone started");

    return 0;

fail_start:
    voip->state = VOIP_PHONE_STATE_ERROR;
    return -1;
}

int voip_phone_start_call(voip_phone_t *voip, const voip_phone_config_t *cfg, const char *call_number)
{
    strncpy(voip->call_number, call_number, sizeof(voip->call_number));
    voip->call_after_register = true;
    return voip_phone_start(voip, cfg);
}

/** Stop VOIP phone */
int voip_phone_stop(voip_phone_t *voip)
{
#if defined(CFG_VOIP_VIDEO_ENABLE) && (CFG_VOIP_VIDEO_ENABLE == 1)
    // Close video stream connection
    if (voip_video_is_active(&voip->video))
        voip_video_close_stream(&voip->video);
#endif

    // Close audio stream connection
    if (voip_audio_is_active(&voip->audio))
        voip_audio_close_stream(&voip->audio);

    sip_destroy_con(&voip->sipcon);
    voip->state = VOIP_PHONE_STATE_IDLE;

    TRACE("SIP phone stopped");

    return 0;
}

/** Outgoing call */
int voip_phone_call(voip_phone_t *voip, const char *call_number)
{
    strncpy(voip->call_number, call_number, sizeof(voip->call_number));

    if (voip->state == VOIP_PHONE_STATE_READY)
    {
        // Make invite
        if (sip_invite(&voip->sipcon, voip->call_number) != 0)
        {
            TRACE_ERROR("Invite call_number: %s", call_number);
            return -1;
        }
        voip->state = VOIP_PHONE_STATE_CONNECTING_OUT;
    }
    else
    {
        TRACE("Hangup current connection before invite");

        // Make invite after hangup current call
        if (sip_hangup(&voip->sipcon) != 0)
        {
            TRACE_ERROR("Hangup");
            return -1;
        }

        voip->state = VOIP_PHONE_STATE_HANGUP_AND_INVITE;
    }

    return 0;
}

int voip_phone_hangup(voip_phone_t *voip)
{
    if (sip_hangup(&voip->sipcon) != 0)
    {
        TRACE_ERROR("Hangup call");
        return -1;
    }

    voip->state = VOIP_PHONE_STATE_HANGUP_PENDING;

    return 0;
}

//-----------------------------------------------------------------------------
//
//                                  SIP events callbacks
//
//-----------------------------------------------------------------------------

/** calling event when connection success registered */
static void on_register(sip_con_t *sc, const sip_profile_t *prof)
{
    voip_phone_t *voip = sc->userData;

    TRACE("SIP event: %s - %s", __FUNCTION__, prof->username);

    if (voip->state == VOIP_PHONE_STATE_WAITFOR_REGISTER)
    {
        voip->state = VOIP_PHONE_STATE_READY;

        if (voip->call_after_register)
        {
            voip->call_after_register = false;
            voip_phone_call(voip, voip->call_number);
        }
    }

    if (voip->events != NULL && voip->events->on_register != NULL)
        voip->events->on_register(voip, prof);
}

/** calling when connection success unregistered */
static void on_unregister(sip_con_t *sc, const sip_profile_t *prof)
{
    voip_phone_t *voip = sc->userData;

    TRACE("SIP event: %s - %s", __FUNCTION__, prof->username);
    voip->state = VOIP_PHONE_STATE_IDLE;

    if (voip->events != NULL && voip->events->on_unregister != NULL)
        voip->events->on_unregister(voip, prof);
}

static void on_invite(sip_con_t *sc, const char *remoteUri, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media)
{
    voip_phone_t *voip = sc->userData;

    TRACE("SIP event: %s - uri:%s  %s:%d  *:%d (media: type=%d  clockRate=%d  bytesPerFrame=%d)",
          __FUNCTION__, remoteUri, remote_addr, remote_port, local_port, sdp_media->audioCodec.type, sdp_media->audioCodec.clockRate, sdp_media->audioCodec.bytesPerFrame);

    // Close audio stream connection
    if (voip_audio_is_active(&voip->audio))
        voip_audio_close_stream(&voip->audio);

    // Open new RTP audio stream
    if (voip_audio_open_stream(&voip->audio, remote_addr, remote_port, local_port, sdp_media) != 0)
    {
        TRACE_ERROR("Open RTP audio stream failed");
        throw_exception(fail);
    }

#if defined(CFG_VOIP_VIDEO_ENABLE) && (CFG_VOIP_VIDEO_ENABLE == 1)
    // Close previous open video stream
    if (voip_video_is_active(&voip->video))
        voip_video_close_stream(&voip->video);

    if (sdp_media->video_port > 0)
    {
        uint32_t sip_uid;

        sip_uid = atoi(voip->cfg->sip_profile.username);

        // Open new transcoding video stream
        if (voip_video_open_transcoding_stream(
                    &voip->video,
                    sip_uid,
                    *sdp_media->video_transcoder_addr != 0 ? sdp_media->video_transcoder_addr : voip->cfg->sip_profile.srv_addr,
                    VIDEO_TRANSCODER_PORT,
                    remote_addr, sdp_media->video_port, VOIP_VIDEO_UDP_PORT,
                    sdp_media) != 0)
        {
            TRACE_ERROR("Open RTP video stream");
            throw_exception(fail);
        }
    }
#endif

    if (voip->state != VOIP_PHONE_STATE_CONNECTED)
    {
        if (voip->state == VOIP_PHONE_STATE_CONNECTING_IN)
        {
            // Incomming call
        }
        else if (voip->state == VOIP_PHONE_STATE_CONNECTING_OUT)
        {
            // Outgoing call
        }

        voip->state = VOIP_PHONE_STATE_CONNECTED;
    }

    if (voip->events != NULL && voip->events->on_invite != NULL)
        voip->events->on_invite(voip, remoteUri, remote_addr, remote_port, local_port, sdp_media);

    return;

fail:
#if defined(CFG_VOIP_VIDEO_ENABLE) && (CFG_VOIP_VIDEO_ENABLE == 1)
    voip_video_close_stream(&voip->video);
#endif
    voip_audio_close_stream(&voip->audio);
    voip->state = VOIP_PHONE_STATE_ERROR;
}

static void on_invite_trying(sip_con_t *sc, const char *remoteUri)
{
    voip_phone_t *voip = sc->userData;
    TRACE("SIP event: %s - %s", __FUNCTION__, remoteUri);
    if (voip->events != NULL && voip->events->on_invite_trying != NULL)
        voip->events->on_invite_trying(voip, remoteUri);
}

static void on_invite_ringing(sip_con_t *sc, const char *remoteUri)
{
    voip_phone_t *voip = sc->userData;
    TRACE("SIP event: %s - %s", __FUNCTION__, remoteUri);
    if (voip->events != NULL && voip->events->on_invite_ringing != NULL)
        voip->events->on_invite_ringing(voip, remoteUri);
}

static void on_incomming_invite_ringing(sip_con_t *sc, const char *remoteUri)
{
    voip_phone_t *voip = sc->userData;

    TRACE("SIP event: %s - %s", __FUNCTION__, remoteUri);

    if (voip->state != VOIP_PHONE_STATE_CONNECTING_IN)
    {
        // Auto accept connection
        sip_accept_invite(sc);

        voip->state = VOIP_PHONE_STATE_CONNECTING_IN;
    }

    if (voip->events != NULL && voip->events->on_incomming_invite_ringing != NULL)
        voip->events->on_incomming_invite_ringing(voip, remoteUri);
}

static void on_invite_declined(sip_con_t *sc, const char *remoteUri)
{
    voip_phone_t *voip = sc->userData;

    TRACE("SIP event: %s - %s", __FUNCTION__, remoteUri);

    voip->state = VOIP_PHONE_STATE_READY;

    if (voip->events != NULL && voip->events->on_invite_declined != NULL)
        voip->events->on_invite_declined(voip, remoteUri);
}

static void on_hangup(sip_con_t *sc, const char *remoteUri)
{
    voip_phone_t *voip = sc->userData;
    voip_phone_state_e prev_state;

    TRACE("SIP event: %s - %s", __FUNCTION__, remoteUri);

#if defined(CFG_VOIP_VIDEO_ENABLE) && (CFG_VOIP_VIDEO_ENABLE == 1)
    // Close video stream connection
    if (voip_video_is_active(&voip->video))
        voip_video_close_stream(&voip->video);
#endif

    // Close audio stream connection
    if (voip_audio_is_active(&voip->audio))
        voip_audio_close_stream(&voip->audio);

    // Store current state
    prev_state = voip->state;

    // Set new state
    voip->state = VOIP_PHONE_STATE_READY;

    switch(prev_state)
    {
        // Make invite
        case VOIP_PHONE_STATE_HANGUP_AND_INVITE:
        {
            TRACE("Make invite after hangup");
            sip_invite(&voip->sipcon, voip->call_number);
            voip->state = VOIP_PHONE_STATE_CONNECTING_OUT;
        }
        break;

        // Hangup from other side
        default:
            break;
    }

    if (voip->events != NULL && voip->events->on_hangup != NULL)
        voip->events->on_hangup(voip, remoteUri);
}

static void on_error(sip_con_t *sc, uint16_t sipErrCode, sip_err_t errcode)
{
    voip_phone_t *voip = sc->userData;

    TRACE_ERROR("SIP event: %s - sip_err_code: %d  errCode: %d", __FUNCTION__, sipErrCode, errcode);

    if (IS_SIP_REGISTER_ERROR(errcode))
    {
        //
        // Registration error
        //

        if (voip->state == VOIP_PHONE_STATE_READY)
        {
            voip->state = VOIP_PHONE_STATE_WAITFOR_REGISTER;
        }

         // Try register
         sip_register(&voip->sipcon, &voip->cfg->sip_profile, 0);
    }
    else
    {
        //
        // Invite error
        //

#if defined(CFG_VOIP_VIDEO_ENABLE) && (CFG_VOIP_VIDEO_ENABLE == 1)
        // Close video stream connection
        if (voip_video_is_active(&voip->video))
            voip_video_close_stream(&voip->video);
#endif

        // Close audio stream connection
        if (voip_audio_is_active(&voip->audio))
            voip_audio_close_stream(&voip->audio);

        switch(voip->state)
        {
        case VOIP_PHONE_STATE_HANGUP_AND_INVITE:
        {
            // Make invite
            sip_invite(&voip->sipcon, voip->call_number);
            voip->state = VOIP_PHONE_STATE_CONNECTING_OUT;
        }
        break;

        default:
            voip->state = VOIP_PHONE_STATE_READY;
        }
    }

    if (voip->events != NULL && voip->events->on_error != NULL)
        voip->events->on_error(voip, sipErrCode, errcode);
}
