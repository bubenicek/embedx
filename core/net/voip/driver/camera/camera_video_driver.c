
#include "system.h"
#include "dev/video/camera/camera.h"
#include "voip_video.h"

#define TRACE_TAG "voip-camera-drv"
#if !ENABLE_TRACE_VOIP_VIDEO
#undef TRACE
#define TRACE(...)
#endif

#define CONFIG_D0       35
#define CONFIG_D1       17
#define CONFIG_D2       34
#define CONFIG_D3       5
#define CONFIG_D4       39
#define CONFIG_D5       18
#define CONFIG_D6       36
#define CONFIG_D7       19
#define CONFIG_XCLK     27
#define CONFIG_PCLK     21
#define CONFIG_VSYNC    22
#define CONFIG_HREF     26
#define CONFIG_SDA      25
#define CONFIG_SCL      23
#define CONFIG_RESET    15


static int camera_driver_open(const voip_video_driver_config_t *cfg)
{
    esp_err_t err;
    camera_model_t camera_model;
    camera_config_t config = {
     .ledc_channel = LEDC_CHANNEL_0,
     .ledc_timer = LEDC_TIMER_0,
     .pin_d0 = CONFIG_D0,
     .pin_d1 = CONFIG_D1,
     .pin_d2 = CONFIG_D2,
     .pin_d3 = CONFIG_D3,
     .pin_d4 = CONFIG_D4,
     .pin_d5 = CONFIG_D5,
     .pin_d6 = CONFIG_D6,
     .pin_d7 = CONFIG_D7,
     .pin_xclk = CONFIG_XCLK,
     .pin_pclk = CONFIG_PCLK,
     .pin_vsync = CONFIG_VSYNC,
     .pin_href = CONFIG_HREF,
     .pin_sscb_sda = CONFIG_SDA,
     .pin_sscb_scl = CONFIG_SCL,
     .pin_reset = CONFIG_RESET,
     .xclk_freq_hz = 0,
    };

    esp_log_level_set("camera", ESP_LOG_DEBUG);
    esp_log_level_set("camera", ESP_LOG_VERBOSE);

    err = camera_probe(&config, &camera_model);
    if (err != ESP_OK)
    {
        TRACE_ERROR("Camera probe failed with error 0x%x", err);
        goto fail_probe;
    }

    if (camera_model == CAMERA_OV7725)
    {
        TRACE("Detected OV7725 camera, using grayscale bitmap format");
        config.pixel_format = CAMERA_PF_GRAYSCALE;
        config.frame_size = cfg->resolution;
    }
    else if (camera_model == CAMERA_OV2640)
    {
        TRACE("Detected OV2640 camera, using JPEG format");
        config.pixel_format = CAMERA_PF_JPEG;
        config.frame_size = cfg->resolution;
        config.jpeg_quality = cfg->jpeg_quality;
    }
    else
    {
        TRACE_ERROR("Camera not supported");
        goto fail_nosuport;
    }

    // Initialize camera
    err = camera_init(&config);
    if (err != ESP_OK)
    {
        TRACE_ERROR("Camera init failed with error 0x%x", err);
        goto fail_init;
    }

    err = camera_run();
    if (err != ESP_OK)
    {
        TRACE_ERROR("Camera run failed with error 0x%x", err);
        goto fail_run;
    }

    TRACE("Camera open, resolution: %d  jpeg_quality: %d", config.frame_size, config.jpeg_quality);

    return 0;

fail_run:
    camera_deinit();
fail_init:
fail_nosuport:
fail_probe:
    return -1;
}

static int camera_driver_close(void)
{
    // Deinitialize camera
    camera_deinit();

    TRACE("Close");

    return 0;
}

static int camera_driver_read(uint8_t *buf, int bufsize, bool *frame_start)
{
    return camera_fb_read(buf, bufsize, portMAX_DELAY, frame_start);
}

const voip_video_driver_t camera_video_driver =
{
    .name = "camera",
    .open = camera_driver_open,
    .close = camera_driver_close,
    .read = camera_driver_read,
};
