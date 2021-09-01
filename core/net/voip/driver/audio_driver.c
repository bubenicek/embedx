
#include "system.h"
#include "audio_driver.h"

#define TRACE_TAG "voip-audio"
#if !ENABLE_TRACE_VOIP_AUDIO
#undef TRACE
#define TRACE(...)
#endif

// Globals:
extern voip_audio_driver_t max9860_audio_driver;

// Locals:
static const voip_audio_driver_t *audio_drivers[] =
{
    &max9860_audio_driver,
    NULL
};

static osMutexId mutex = NULL;

/** Initialize audio driver framework */
int audio_driver_init(void)
{
    if (mutex == NULL)
    {
        if ((mutex = osMutexCreate(NULL)) == NULL)
        {
            TRACE_ERROR("Create mutex failed");
            return -1;
        }

        TRACE("Audio drivers init");
    }

    return 0;
}

/** Open audio driver */
const voip_audio_driver_t *audio_driver_open(const char *name)
{
    int ix = 0;
    const voip_audio_driver_t *drv = NULL;

    ASSERT(mutex != NULL);

    for (ix = 0; audio_drivers[ix] != NULL; ix++)
    {
        if (!strcmp(audio_drivers[ix]->name, name))
        {
            drv = audio_drivers[ix];
            break;
        }
    }

    if (drv != NULL)
    {
        osMutexWait(mutex, osWaitForever);

        if (drv->open() != 0)
        {
            osMutexRelease(mutex);
            TRACE_ERROR("Open audio driver %s failed", name);
            return NULL;
        }

        osMutexRelease(mutex);
    }

    return drv;
}

/** Close audio driver */
int audio_driver_close(const voip_audio_driver_t *drv)
{
    int res;

    ASSERT(mutex != NULL);

    osMutexWait(mutex, osWaitForever);
    res = drv->close();
    osMutexRelease(mutex);

    return res;
}

/** Read samples */
int audio_driver_read(const voip_audio_driver_t *drv, int16_t *samples, int samples_count)
{
    int res;

    ASSERT(mutex != NULL);

    osMutexWait(mutex, osWaitForever);
    res = drv->read_samples(samples, samples_count);
    osMutexRelease(mutex);

    return res;
}

/** Write audio samples */
int audio_driver_write(const voip_audio_driver_t *drv, int16_t *samples, int samples_count)
{
    int res;

    ASSERT(mutex != NULL);

    osMutexWait(mutex, osWaitForever);
    res = drv->write_samples(samples, samples_count);
    osMutexRelease(mutex);

    return res;
}
