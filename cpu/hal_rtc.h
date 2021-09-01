
#ifndef __HAL_RTC_H
#define __HAL_RTC_H

#include <time.h>

/** Alarm callback function type */
typedef void (*hal_rtc_time_cb_t)(void *arg);

/** RTC Alarm */
typedef struct hal_rtc_alarm
{
   struct hal_rtc_alarm *next;
   time_t time;
   hal_rtc_time_cb_t cb;
   void *arg;

} hal_rtc_alarm_t;


/** Init RTC */
int hal_rtc_init(void);

/** Deinitialize RTC */
int hal_rtc_deinit(void);

/** Set RTC time in seconds from epoch 1970-01-01 00:00:00 +0000 (UTC) */
int hal_rtc_set_time(time_t time);

/** Get RTC time in seconds from epoch 1970-01-01 00:00:00 +0000 (UTC). */
int hal_rtc_get_time(time_t *time);

/** Register RTC alarm */
int hal_rtc_add_alarm(hal_rtc_alarm_t *alarm, time_t alarm_time, hal_rtc_time_cb_t alarm_cb, void *arg);

/** Unregister RTC alarm */
int hal_rtc_remove_alarm(hal_rtc_alarm_t *alarm);


#endif // __HAL_RTC_H
