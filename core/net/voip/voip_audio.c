/**
 * \file voip_audio.c           \brief VOIP RTP audio callback impl.
 */

#include "system.h"
#include "voip_audio.h"
#include "g711/alaw.h"

#define TRACE_TAG "voip-audio"
#if !ENABLE_TRACE_VOIP_AUDIO
#undef TRACE
#define TRACE(...)
#endif


// Prototypes:
static uint16_t on_rtp_send_frame_data(rtp_connection_t *rcon, uint8_t *buf, uint16_t buflen, void *user_param);
static void on_rtp_receive_frame_data(rtp_connection_t *con, const pjmedia_rtp_hdr *hdr, uint8_t *buf, uint16_t count, void *user_param);
static void on_rtp_receive_comfort_noise(rtp_connection_t *con, void *user_param);
static void on_rtp_receive_dtmf(rtp_connection_t *con, rtp_dtmf_code_t code, uint16_t duration, uint16_t volume, void *user_param);
static void on_rtp_sent_dtmf(rtp_connection_t *con, rtp_dtmf_code_t code, void *user_param);


/** RTP callbacks events */
static const rtp_connection_events_t rtp_events =
{
    .on_send_frame_data = on_rtp_send_frame_data,
    .on_receive_frame_data = on_rtp_receive_frame_data,
    .on_receive_dtmf = on_rtp_receive_dtmf,
    .on_sent_dtmf = on_rtp_sent_dtmf,
    .on_receive_comfort_noise = on_rtp_receive_comfort_noise,

};

//-----------------------------------------------------------------------------

int voip_audio_open_stream(voip_audio_t *audio, const char *remote_addr, uint16_t remote_port, uint16_t local_port, const sip_sdp_media_t *sdp_media)
{
    voip_ipaddr_t remoteIpAddr;
    audio_codec_type_e acodec_type;

    audio->sdp_media = sdp_media;
    audio->sample_new = audio->sample_old = 0;

    // Open audio codec
    switch(sdp_media->audioCodec.type)
    {
        case RTP_PCMA_PAYLOADTYPE:
            acodec_type = AUDIO_CODEC_TYPE_G711;
            break;

        case RTP_CODEC2_PAYLOADTYPE:
            acodec_type = AUDIO_CODEC_TYPE_CODEC2;
            break;

        default:
            TRACE_ERROR("Not supported audio codec type: %d", sdp_media->audioCodec.type);
            return -1;
    }

    // Open audio codec
    if (audio_codec_open(&audio->codec, acodec_type) != 0)
    {
        TRACE_ERROR("Open audio code type: %d", acodec_type);
        goto fail_open_codec;
    }

    // Open audio input/output driver
    if ((audio->audio_driver = audio_driver_open(CFG_VOIP_AUDIO_DRIVER_NAME)) == NULL)
    {
        TRACE_ERROR("Audio driver %s open failed", CFG_VOIP_AUDIO_DRIVER_NAME);
        goto fail_open_audio;
    }

    // Make IP address from string
    if (voip_ipaddr_aton(remote_addr, &remoteIpAddr) != 0)
    {
        TRACE_ERROR("Not valid remote ipaddr: %s", remote_addr);
        goto fail_aton;
    }

    // Create RTP connection
    if (rtp_create_connection(
                &audio->rtpcon,
                &remoteIpAddr,
                remote_port,
                local_port,
                sdp_media->audioCodec.type,
                audio->codec.params.bytes_per_frame,
                sdp_media->audioCodec.clockRate,
                sdp_media->telephone_event,
                rand(),
                &rtp_events,
                audio) != 0)
    {
        TRACE_ERROR("Create RTP connection failed");
        goto fail_create_rtpcon;
    }

    return 0;

    rtp_destroy_connection(&audio->rtpcon);
fail_create_rtpcon:
fail_aton:
    audio_driver_close(audio->audio_driver);
fail_open_audio:
    audio_codec_close(&audio->codec);
fail_open_codec:
    return -1;
}

int voip_audio_close_stream(voip_audio_t *audio)
{
    // Destroy RTP connection
    rtp_destroy_connection(&audio->rtpcon);

    // Close audiocodec
    audio_codec_close(&audio->codec);

    // Close audio driver
    audio_driver_close(audio->audio_driver);

    return 0;
}

//-----------------------------------------------------------------------------
//
//                                  RTP callbacks
//
//-----------------------------------------------------------------------------


/** Send audio samples */
static uint16_t on_rtp_send_frame_data(rtp_connection_t *rcon, uint8_t *buf, uint16_t buflen, void *user_param)
{
    int ix, res = 0;
    voip_audio_t *audio = user_param;
    int16_t *psamples = audio->samples;
    int samples_count;

    ASSERT(audio->codec.params.samples_per_frame <= CFG_VOIP_AUDIO_SAMPLES_BUFSIZE);

    // Read samples from audio input
    if ((samples_count = audio_driver_read(audio->audio_driver, audio->samples, audio->codec.params.samples_per_frame * 2)) > 0)
    {
        // Downsample from 16KHz to 8Khz
        for (ix = 0; ix < samples_count; ix++)
        {
            if (ix & 0x01)
            {
                *psamples++ = audio->samples[ix];
            }
        }
        samples_count /= 2;

        // Encode samples
        res = audio_codec_encode(&audio->codec, audio->samples, samples_count, buf, buflen);
    }

    return res;
}


/** Receive audio samples */
static void on_rtp_receive_frame_data(rtp_connection_t *rcon, const pjmedia_rtp_hdr *hdr, uint8_t *buf, uint16_t count, void *user_param)
{
    int ix;
    static int16_t samples2[CFG_VOIP_AUDIO_SAMPLES_BUFSIZE];
    int16_t *psamples2 = samples2;
    int samples2_count = 0;
    int samples_count = 0;
    voip_audio_t *audio = user_param;

    // Decode to samples
    if ((samples2_count = audio_codec_decode(&audio->codec, buf, count, samples2, CFG_VOIP_AUDIO_SAMPLES_BUFSIZE)) > 0)
    {
        // Copy interpolated samples to DAC (8KHz to 16KHz)
        for (ix = 0; ix < samples2_count * 2; ix++)
        {
            if (ix & 0x1)
            {
                audio->sample_new = *psamples2++;
                audio->samples[samples_count++] = audio->sample_old;
            }
            else
            {
                audio->samples[samples_count++] = (audio->sample_old / 2) + (audio->sample_new / 2);
                audio->sample_old = audio->sample_new;
            }
        }

        // Write to audio output
        audio_driver_write(audio->audio_driver, audio->samples, samples_count);
    }
}

static void on_rtp_receive_comfort_noise(rtp_connection_t *rcon, void *user_param)
{
}

static void on_rtp_receive_dtmf(rtp_connection_t *rcon, rtp_dtmf_code_t code, uint16_t duration, uint16_t volume, void *user_param)
{
    TRACE("Recv RTP DTMF code = %d ('%c')  duration = %d   volume = %d", code, rtp_dtmfcode_to_ascii(code), duration, volume);
}

static void on_rtp_sent_dtmf(rtp_connection_t *rcon, rtp_dtmf_code_t code, void *user_param)
{
}
