
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "voip_platform.h"
#include "../audio_codec.h"
#include "codec2/codec2.h"


#define DEBUG_TAG "audio-codec-c2"

#if ! DEBUG_AUDIO_CODEC_C2
#undef voip_debug
#define voip_debug(...)
#endif

#define VOIP_CODEC2_MODE        CODEC2_MODE_2400


static int audio_codec2_open(audio_codec_t *codec)
{
    struct CODEC2 *c2;

    c2 = codec2_create(VOIP_CODEC2_MODE);
    if (c2 == NULL)
    {
        voip_error("Create C2 codec");
        return -1;
    }

    // Set codec params
    codec->params.samples_per_frame = codec2_samples_per_frame(c2);
    codec->params.bytes_per_frame = (codec2_bits_per_frame(c2) + 7) / 8;

    voip_debug("Codec C2 opened, samples_per_frame: %d   bytes_per_frame: %d", codec->params.samples_per_frame, codec->params.bytes_per_frame);

    codec->private_data = c2;

    return 0;
}

static int audio_codec2_close(audio_codec_t *codec)
{
    struct CODEC2 *c2 = codec->private_data;

    ASSERT(c2 != NULL);

    codec2_destroy(c2);
    codec->private_data = NULL;

    voip_debug("Codec C2 closed");

    return 0;
}

static int audio_codec2_encode(audio_codec_t *codec, int16_t *samples_buf, int samples_count, uint8_t *bufout, int bufout_length)
{
    struct CODEC2 *c2 = codec->private_data;

    ASSERT(c2 != NULL);
    ASSERT(samples_count == codec->params.samples_per_frame);
    ASSERT(bufout_length == codec->params.bytes_per_frame);

    codec2_encode(c2, bufout, samples_buf);

    return 0;
}

static int audio_codec2_decode(audio_codec_t *codec, uint8_t *bufin, int bufin_length, int16_t *samples_buf, int samples_buf_size)
{
    struct CODEC2 *c2 = codec->private_data;

    ASSERT(c2 != NULL);
    ASSERT(samples_buf_size == codec->params.samples_per_frame);
    ASSERT(bufin_length == codec->params.bytes_per_frame);

    codec2_decode(c2, samples_buf, bufin);

    return 0;
}

const audio_codec_driver_t audio_codec_codec2 =
{
    .type = AUDIO_CODEC_TYPE_CODEC2,
    .open = audio_codec2_open,
    .close = audio_codec2_close,
    .encode = audio_codec2_encode,
    .decode = audio_codec2_decode
};

