
#ifndef __XCLK_H
#define __XCLK_H

/** Enable XCLK */
esp_err_t xclk_enable_out(uint32_t xclk_freq_hz, uint32_t pin_xclk);

/** Disable XCLK */
esp_err_t xclk_disable_out_clock(void);

#endif   // __XCLK_H
