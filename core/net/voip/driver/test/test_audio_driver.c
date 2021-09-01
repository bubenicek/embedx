
#include "system.h"
#include "voip_audio.h"


#define TRACE_TAG "voip-audio"
#if !ENABLE_TRACE_VOIP_AUDIO
#undef TRACE
#define TRACE(...)
#endif

//
// Audio input driver
//

/** sinwave lookup table, 16 samples (Fs = 16kHz) */
static const int16_t sinwave[] =
{
0,
12539,
23170,
30273,
32767,
30273,
23170,
12539,
0,
-12539,
-23170,
-30273,
-32767,
-30273,
-23170,
-12539,
};
#define SINWAVE_NUM_SAMPLES     16

static int sinwave_index;

int audio_open(void)
{
    sinwave_index = 0;
    TRACE("%s", __FUNCTION__);
    return 0;
}

int audio_close(void)
{
    TRACE("%s", __FUNCTION__);
    return 0;
}

int audio_input_read(int16_t *samples, int samples_count)
{
    int total;

    for (total = 0; samples_count > 0; samples_count--, total++)
    {
        *samples++ = sinwave[sinwave_index++];
        if (sinwave_index == SINWAVE_NUM_SAMPLES)
            sinwave_index = 0;
    }

    return total;
}

int audio_input_samples_count(void)
{
    return CFG_VOIP_AUDIO_INPUT_QUEUE_SIZE / 2;
}

int audio_output_write(int16_t *samples, int samples_count)
{
    return samples_count;
}

int audio_output_samples_count(void)
{
    return CFG_VOIP_AUDIO_OUTPUT_QUEUE_SIZE / 2;
}
