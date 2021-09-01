/**
 * \file voip_audio.h           \brief VOIP driver RTP callback impl.
 */

#ifndef __voip_audio_h
#define __voip_audio_h

#include "voip.h"

#ifndef CFG_VOIP_AUDIO_SAMPLES_BUFSIZE
#define CFG_VOIP_AUDIO_SAMPLES_BUFSIZE     512
#endif

typedef struct
{
    /** RTP connection */
    rtp_connection_t rtpcon;

    /** Audio codec */
    audio_codec_t codec;

    /** Audio driver */
    const voip_audio_driver_t *audio_driver;

    /** Audio settings */
    const sip_sdp_media_t *sdp_media;

    /** Resampling */
    int16_t sample_new, sample_old;
    int16_t samples[CFG_VOIP_AUDIO_SAMPLES_BUFSIZE];

} voip_audio_t;


/** Open audio RTP stream */
int voip_audio_open_stream(voip_audio_t *audio, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media);

/** Close audio RTP stream */
int voip_audio_close_stream(voip_audio_t *audio);

/** Return true if audio is active */
#define voip_audio_is_active(_audio) \
    rtp_is_active(&(_audio)->rtpcon)


#endif   // voip_audio.h
