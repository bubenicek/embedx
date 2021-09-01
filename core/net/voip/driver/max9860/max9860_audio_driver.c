/**
 * \file max9860_audio_driver.c     \brief MAX9860 audio driver
 */

#include "system.h"
#include "voip_audio.h"
#include "max9860.h"
#include "wiring.h"

#define TRACE_TAG "voip-audio"
#if !ENABLE_TRACE_VOIP_AUDIO
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_GPIO_POWEN
#define CFG_GPIO_POWEN                  GPIO_NUM_16
#endif

static int refcnt = 0;

static int max9860_driver_open(void)
{
    if (!refcnt)
    {
        // Enable power
        pinMode(CFG_GPIO_POWEN, OUTPUT);
        digitalWrite(CFG_GPIO_POWEN, 1);

        TRACE("Audio power enabled");

        if (max9860_init() != 0)
        {
            TRACE_ERROR("max9860 init failed");
            return -1;
        }
    }

    refcnt++;
    TRACE("Open max9860 refcnt: %d", refcnt);

    return 0;
}

static int max9860_driver_close(void)
{
    refcnt--;

    if (!refcnt)
    {
        max9860_deinit();

        // Disable power
        pinMode(CFG_GPIO_POWEN, OUTPUT);
        digitalWrite(CFG_GPIO_POWEN, 0);

        TRACE("Audio power disabled");
    }

    TRACE("Close max9860 refcnt: %d", refcnt);

    return 0;
}

static int max9860_driver_read_samples(int16_t *samples, int samples_count)
{
    int res;

    if ((res = max9860_read_input((uint8_t *)samples, samples_count * 2)) > 0)
        res /= 2;

    return res;
}

static int max9860_driver_write_samples(int16_t *samples, int samples_count)
{
    int res;

    if ((res = max9860_write_output((uint8_t *)samples, samples_count * 2)) > 0)
        res /= 2;

    return res;
}

const voip_audio_driver_t max9860_audio_driver =
{
    .name = "max9860",
    .sample_rate = CFG_MAX9860_SAMPLERATE,
    .bits_per_sample = CFG_MAX9860_BITSPERSAMPLE,
    .open = max9860_driver_open,
    .close = max9860_driver_close,
    .read_samples = max9860_driver_read_samples,
    .write_samples = max9860_driver_write_samples
};
