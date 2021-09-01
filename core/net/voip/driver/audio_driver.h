
#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H


/** Audio driver interface */
typedef struct
{
    /** Audio driver name */
    char *name;

    /** Audio driver sample rate */
    uint16_t sample_rate;

    /** Number of bits per sample */
    uint16_t bits_per_sample;

    /** Open audio driver */
    int (*open)(void);

    /** Close audop driver */
    int (*close)(void);

    /** Read input samples */
    int (*read_samples)(int16_t *samples, int samples_count);

    /** Write output samples */
    int (*write_samples)(int16_t *samples, int samples_count);

} voip_audio_driver_t;


/** Initialize audio driver framework */
int audio_driver_init(void);

/** Open audio driver */
const voip_audio_driver_t *audio_driver_open(const char *name);

/** Close audio driver */
int audio_driver_close(const voip_audio_driver_t *drv);

/** Read samples */
int audio_driver_read(const voip_audio_driver_t *drv, int16_t *samples, int samples_count);

/** Write audio samples */
int audio_driver_write(const voip_audio_driver_t *drv, int16_t *samples, int samples_count);


#endif   // __AUDIO_DRIVER_IN_H
