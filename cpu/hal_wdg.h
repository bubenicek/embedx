
#ifndef __HAL_WDG_H
#define __HAL_WDG_H

#if defined(CFG_HAL_WDG_ENABLED) && (CFG_HAL_WDG_ENABLED == 1)

int hal_wdg_init(void);
void hal_wdg_reset();
uint8_t hal_wdg_reset_occured();

#else

#define hal_wdg_init()
#define hal_wdg_reset()
#define hal_wdg_reset_occured() 0

#endif // CFG_HAL_WDG_ENABLED

#endif // __HAL_WDG_H
