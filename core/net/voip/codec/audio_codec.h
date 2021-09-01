
#ifndef __audio_codec_h
#define __audio_codec_h


/** Audio codec types */
typedef enum
{
    AUDIO_CODEC_TYPE_G711,
    AUDIO_CODEC_TYPE_CODEC2

} audio_codec_type_e;


/** Audio codec parameters */
typedef struct
{
    /** 16bit linear samples per frame */
    int samples_per_frame;

    /** Encoded bytes per frame */
    int bytes_per_frame;

} audio_codec_params_t;



/** Audio codec */
typedef struct
{
    /** Ptr to driver functions */
    const struct audio_codec_driver *drv;

    /** Codec parameters */
    audio_codec_params_t params;

    /** Private codec data */
    void *private_data;

} audio_codec_t;


/** VOIP audio codec driver */
typedef struct audio_codec_driver
{
    audio_codec_type_e type;

    /** Open codec */
    int (*open)(audio_codec_t *codec);

    /** Close codec */
    int (*close)(audio_codec_t *codec);

    /** Encode linear 16bit samples frame to codec format */
    int (*encode)(audio_codec_t *codec, int16_t *samples_buf, int samples_count, uint8_t *bufout, int bufout_length);

    /** Decode from codec frame format to linear 16bit samples */
    int (*decode)(audio_codec_t *codec, uint8_t *bufin, int bufin_length, int16_t *samples_buf, int samples_buf_size);

} audio_codec_driver_t;


/** Open audio codec */
int audio_codec_open(audio_codec_t *codec, audio_codec_type_e type);

/** Close audio codec */
#define audio_codec_close(_codec) \
    (_codec)->drv->close(_codec)

/** Encode 16bit linear data -> codec */
#define audio_codec_encode(_codec, _samples_buf, _samples_count, _bufout, _bufout_length) \
    (_codec)->drv->encode(_codec, _samples_buf, _samples_count, _bufout, _bufout_length)

/** Decode codec data -> linear 16bit data */
#define audio_codec_decode(_codec, _bufin, _bufin_length, _samples_buf, _samples_buf_size) \
    (_codec)->drv->decode(_codec, _bufin, _bufin_length, _samples_buf, _samples_buf_size)


#endif   // audio_codec.h
