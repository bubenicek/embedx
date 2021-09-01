
#include "system.h"
#include "voip_video.h"

#include "monoskop_320x240_1.h"
#include "monoskop_320x240_2.h"
#include "monoskop_320x240_3.h"
#include "monoskop_320x240_4.h"
#include "monoskop_320x240_5.h"

#define TRACE_TAG "voip-video-drv"
#if !ENABLE_TRACE_VOIP_VIDEO
#undef TRACE
#define TRACE(...)
#endif


#define SEQ_TIME_MS     1000

typedef struct
{
    const char *data;
    int length;

} video_sequence_t;

static video_sequence_t seq[] =
{
    {monoskop_320x240_1, sizeof(monoskop_320x240_1)-1},
    {monoskop_320x240_2, sizeof(monoskop_320x240_2)-1},
    {monoskop_320x240_3, sizeof(monoskop_320x240_3)-1},
    {monoskop_320x240_4, sizeof(monoskop_320x240_4)-1},
    {monoskop_320x240_5, sizeof(monoskop_320x240_5)-1},
};
#define NUM_SEQ (sizeof(seq) / sizeof(video_sequence_t))


static int seq_offset;
static int seq_index;
static uint32_t seq_time;

static int test_driver_open(const voip_video_driver_config_t *cfg)
{
    seq_offset = 0;
    seq_index = 0;
    seq_time = hal_time_ms() + SEQ_TIME_MS;

    TRACE("Open");

    return 0;
}

static int test_driver_close(void)
{
    TRACE("Close");
    return 0;
}

static int test_driver_read(uint8_t *buf, int bufsize, bool *frame_start)
{
    int len;

    *frame_start = (seq_offset == 0);

    len = bufsize > (seq[seq_index].length - seq_offset) ? seq[seq_index].length - seq_offset : bufsize;
    memcpy(buf, &seq[seq_index].data[seq_offset], len);
    seq_offset += len;

    if (seq_offset == seq[seq_index].length)
    {
        seq_offset = 0;

        if (hal_time_ms() >= seq_time)
        {
            if (++seq_index == NUM_SEQ)
                seq_index = 0;

            seq_time = hal_time_ms() + SEQ_TIME_MS;
        }
    }

    return len;
}

const voip_video_driver_t test_video_driver =
{
    .name = "test-jpeg",
    .open = test_driver_open,
    .close = test_driver_close,
    .read = test_driver_read,
};

