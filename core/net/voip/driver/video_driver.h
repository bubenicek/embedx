
#ifndef __VIDEO_DRIVER_H
#define __VIDEO_DRIVER_H


/** Configuration */
typedef struct
{
    uint32_t resolution;
    uint32_t jpeg_quality;

} voip_video_driver_config_t;


/** Video driver interface */
typedef struct
{
    /** Driver name */
    char *name;

    /** Open audio driver */
    int (*open)(const voip_video_driver_config_t *cfg);

    /** Close audop driver */
    int (*close)(void);

    /** Read input samples */
    int (*read)(uint8_t *buf, int bufsize, bool *frame_start);

} voip_video_driver_t;


/** Open video driver */
const voip_video_driver_t *video_driver_open(const char *name, const voip_video_driver_config_t *cfg);

/** Close video driver */
#define video_driver_close(_drv) \
    (_drv)->close()

/** Read samples */
#define video_driver_read(_drv, _buf, _bufsize, _frame_start) \
    (_drv)->read(_buf, _bufsize, _frame_start)


#endif   // __AUDIO_DRIVER_IN_H
