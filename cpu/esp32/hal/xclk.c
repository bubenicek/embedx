
#include "system.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "xclk.h"

TRACE_TAG(xclk);

esp_err_t xclk_enable_out(uint32_t xclk_freq_hz, uint32_t pin_xclk)
{
    esp_err_t res;

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num  = LEDC_TIMER_0,
        .bit_num    = 1,
        .freq_hz    = xclk_freq_hz
    };

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .gpio_num   = pin_xclk,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 1
    };

    res = ledc_timer_config(&ledc_timer);
    if (res != ESP_OK)
    {
      TRACE_ERROR("ledc_timer_config failed, res=%x", res);
      return res;
    }

    res = ledc_channel_config(&ledc_channel);
    if (res != ESP_OK)
    {
      TRACE_ERROR("ledc_channel_config failed, res=%x", res);
      return res;
    }

    TRACE("Enable XCLK=%d Hz gpio: %d", xclk_freq_hz, pin_xclk);

    return res;
}

esp_err_t xclk_disable_out_clock(void)
{
   periph_module_disable(PERIPH_LEDC_MODULE);
   return ESP_OK;
}
