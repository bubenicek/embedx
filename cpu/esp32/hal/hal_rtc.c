#include "system.h"

TRACE_TAG(hal_rtc);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

/** Init RTC */
int hal_rtc_init(void)
{
   return 0;
}

/** Deinitialize RTC */
int hal_rtc_deinit(void)
{
   return 0;
}

/** Set RTC time in seconds from epoch 1970-01-01 00:00:00 +0000 (UTC) */
int hal_rtc_set_time(time_t t)
{
   struct timeval tv = {.tv_sec = t, .tv_usec = 0};

   if (settimeofday(&tv, NULL) != 0)
   {
      TRACE_ERROR("settimeofday failed");
      return -1;
   }

   return 0;
}

/** Get RTC time in seconds from epoch 1970-01-01 00:00:00 +0000 (UTC). */
int hal_rtc_get_time(time_t *t)
{
   if (time(t) < 0)
      return -1;

   return 0;
}

