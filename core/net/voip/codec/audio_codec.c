
#include "voip_platform.h"
#include "audio_codec.h"

#define TRACE_TAG "audiocodec"
#if !ENABLE_TRACE_AUDIOCODEC
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_AUDIOCODEC_G711
#define CFG_AUDIOCODEC_G711     1
#endif

#ifndef CFG_AUDIOCODEC_CODEC2
#define CFG_AUDIOCODEC_CODEC2    0
#endif


//
// Available audio codecs
//
extern const audio_codec_driver_t audio_codec_g711;
extern const audio_codec_driver_t audio_codec_codec2;


static const audio_codec_driver_t *audio_codecs[] =
{
#if defined(CFG_AUDIOCODEC_G711) && (CFG_AUDIOCODEC_G711 == 1)
    &audio_codec_g711,
#endif
#if defined(CFG_AUDIOCODEC_CODEC2) && (CFG_AUDIOCODEC_CODEC2 == 1)
    &audio_codec_codec2,
#endif
};

#define NUM_CODECS      (sizeof(audio_codecs) / sizeof(audio_codec_driver_t *))


int audio_codec_open(audio_codec_t *codec, audio_codec_type_e type)
{
    int ix;

    memset(codec, 0, sizeof(audio_codec_t));

    for (ix = 0; ix < NUM_CODECS; ix++)
    {
        if (audio_codecs[ix]->type == type)
        {
            codec->drv = audio_codecs[ix];
            break;
        }
    }

    if (codec->drv == NULL)
    {
         TRACE_ERROR("Undefined audio codec driver type: %d", type);
         return -1;
    }

    // Test codec driver interface
    ASSERT(codec->drv->open != NULL);
    ASSERT(codec->drv->close != NULL);
    ASSERT(codec->drv->encode != NULL);
    ASSERT(codec->drv->decode != NULL);

    return codec->drv->open(codec);
}

