
#include "system.h"
#include "video_driver.h"

#define TRACE_TAG "voip-video"
#if !ENABLE_TRACE_VOIP_VIDEO
#undef TRACE
#define TRACE(...)
#endif

// Video drivers
extern const voip_video_driver_t test_video_driver;
extern const voip_video_driver_t camera_video_driver;

/** Available video drivers */
static const voip_video_driver_t *video_drivers[] =
{
    &camera_video_driver,
    &test_video_driver,
    NULL
};

/** Open video driver */
const voip_video_driver_t *video_driver_open(const char *name, const voip_video_driver_config_t *cfg)
{
    int ix;
    const voip_video_driver_t *v = NULL;

    for (ix = 0; video_drivers[ix] != NULL; ix++)
    {
        if (!strcmp(video_drivers[ix]->name, name))
        {
            v = video_drivers[ix];
            break;
        }
    }

    if (v != NULL)
    {
        if (v->open(cfg) != 0)
        {
            TRACE_ERROR("Open video driver %s failed", name);
            return NULL;
        }
    }

    return v;
}


