/**
 * \file audio_codec_g711.c          \brief G711 alaw audio codec
 */

#include <stdint.h>
#include <string.h>

#include "voip_platform.h"
#include "../audio_codec.h"
#include "alaw.h"


#define TRACE_TAG "codec-g711"
#if !ENABLE_TRACE_AUDIOCODEC
#undef TRACE
#define TRACE(...)
#endif


#define CODEC_G711_SAMPLES_PER_FRAME        160
#define CODEC_G711_BYTES_PER_FRAME          160


static int codec_g711_open(audio_codec_t *codec)
{
    // Set codec params
    codec->params.samples_per_frame = CODEC_G711_SAMPLES_PER_FRAME;
    codec->params.bytes_per_frame = CODEC_G711_BYTES_PER_FRAME;

    TRACE("Codec G711 opened, samples_per_frame: %d   bytes_per_frame: %d", codec->params.samples_per_frame, codec->params.bytes_per_frame);

    return 0;
}

static int codec_g711_close(audio_codec_t *codec)
{
    TRACE("%s", __FUNCTION__);

    return 0;
}

static int codec_g711_encode(audio_codec_t *codec, int16_t *samples_buf, int samples_count, uint8_t *bufout, int bufout_length)
{
    int buflen = 0;

    if (bufout_length < samples_count)
    {
        TRACE_ERROR("Output buffer overflow  %d < %d", bufout_length, samples_count);
        return -1;
    }

    while(samples_count > 0)
    {
        *bufout++ = linear_to_alaw(*samples_buf++);
        samples_count--;
        buflen++;
    }

    return buflen;
}

static int codec_g711_decode(audio_codec_t *codec, uint8_t *bufin, int bufin_length, int16_t *samples_buf, int samples_buf_size)
{
    int samples_count = 0;

    if (bufin_length > samples_buf_size * 2)
    {
        TRACE_ERROR("Samples buffer overflow");
        return -1;
    }

    while(bufin_length > 0)
    {
        *samples_buf++ = alaw_to_linear(*bufin++);
        samples_count++,
        bufin_length--;
    }

    return samples_count;
}

const audio_codec_driver_t audio_codec_g711 =
{
    .type = AUDIO_CODEC_TYPE_G711,
    .open = codec_g711_open,
    .close = codec_g711_close,
    .encode = codec_g711_encode,
    .decode = codec_g711_decode
};

