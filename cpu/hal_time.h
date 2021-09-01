
#ifndef __HAL_TIME_H
#define __HAL_TIME_H

typedef uint64_t hal_time_t;

/** Initialize HAL time driver */
int hal_time_init(void);

/** Deinitialize HAL time driver */
int hal_time_deinit(void);

/** Get current time in ms from system start */
hal_time_t hal_time_ms(void);

/** Delay ms */
void hal_delay_ms(hal_time_t ms);

/** Delay usec */
void hal_delay_us(hal_time_t us);

/** Hal timer callback */
extern void hal_timer_cb(void);


#endif // __HAL_TIME_H
