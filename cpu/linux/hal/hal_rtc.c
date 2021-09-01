/**
 * \file hal_rtc.c          \brief RTC implementation for Linux
 */

#include "system.h"

#include <linux/rtc.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>


#if ENABLE_TRACE_HAL
TRACE_TAG(hal_rtc);
#else
#include "trace_undef.h"
#endif

#ifndef CFG_RTC_DEVICE_NAME
#define CFG_RTC_DEVICE_NAME         "/dev/rtc"
#endif

#ifndef CFG_RTC_THREAD_TICK_SEC
#define CFG_RTC_THREAD_TICK_SEC     1
#endif 


// Prototypes:
static void *rtc_thread(void *arg);

// Locals:

/** RTC device descriptor */
static int rtc_fd = -1;

/** handle to RTC thread */
static pthread_t h_thread;

/** thread termination flag */
static uint8_t terminate_thread = 0;



/** Initialize RTC */
int hal_rtc_init(void)
{
   // Open RTC device
   if ((rtc_fd = open(CFG_RTC_DEVICE_NAME, O_RDWR)) < 0)
   {
      TRACE_ERROR("open RTC device '%s'", CFG_RTC_DEVICE_NAME);
      return -1;
   }

   // Disable alarm interrupts
   if (ioctl(rtc_fd, RTC_AIE_OFF, 0) < 0)
   {
      TRACE_ERROR("rtc ioctl RTC_ALE_OFF error");
      throw_exception(fail);
   }

   // Start RTC thread
   if (pthread_create(&h_thread, NULL, rtc_thread, NULL) != 0)
   {
      TRACE("Create RTC thread");
      throw_exception(fail);
   }
   
   TRACE("RTC init");

   return 0;

fail:
   if (rtc_fd != -1)
      close(rtc_fd);
      
   return -1;
}

/** finalize RTC */
int hal_rtc_deinit(void)
{
   // Terminate RTC thread
   terminate_thread = 1;
   pthread_join(h_thread, NULL);
   close(rtc_fd);

   return 0;;
}

/** set RTC date */
int hal_rtc_set_time(time_t time)
{
   struct rtc_time rtc_tm;
   struct tm *tm;
   
   tm = localtime(&time);

   // Set the RTC time/date
   rtc_tm.tm_mday = tm->tm_mday;
   rtc_tm.tm_mon = tm->tm_mon; // - 1;
   rtc_tm.tm_year = tm->tm_year; // - 1900;
   rtc_tm.tm_hour = tm->tm_hour;
   rtc_tm.tm_min = tm->tm_min;
   rtc_tm.tm_sec = tm->tm_sec;

   // Set RTC Time
   if (ioctl(rtc_fd, RTC_SET_TIME, &rtc_tm) < 0)
   {
      TRACE_ERROR("rtc ioctl RTC_SET_TIME error");
      return -1;
   }

   return 0;
}

/** Get RTC date */
int hal_rtc_get_time(time_t *tm)
{
   *tm = time(0);
/*   
   struct rtc_time rtc_tm;

   // Get RTC Time
   if (ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm) < 0)
   {
      error("rtc ioctl RTC_RD_TIME error");
      return LIBWRAP_RET_ERR;
   }

   tm->day = rtc_tm.tm_mday;
   tm->month = rtc_tm.tm_mon + 1;
   tm->year = rtc_tm.tm_year + 1900;
   tm->hour = rtc_tm.tm_hour;
   tm->min = rtc_tm.tm_min;
   tm->sec = rtc_tm.tm_sec;
   tm->msec = 0;
   tm->wday = libwrap_getwday(tm);
*/
   return 0;
}

/** Set allarm callback invoked at specified datetime */
int hal_rtc_add_alarm(hal_rtc_alarm_t *alarm, time_t alarm_time, hal_rtc_time_cb_t alarm_cb, void *arg)
{
/*   
   struct rtc_time rtc_tm;

   // disable alarm interrupts
   if (ioctl(rtc_fd, RTC_AIE_OFF, 0) < 0)
   {
      error("rtc ioctl RTC_ALE_OFF error");
      return LIBWRAP_RET_ERR;
   }

   // set alarm time
   rtc_tm.tm_mday = tm->day;
   rtc_tm.tm_mon = tm->month - 1;
   rtc_tm.tm_year = tm->year - 1900;
   rtc_tm.tm_hour = tm->hour;
   rtc_tm.tm_min = tm->min;
   rtc_tm.tm_sec = tm->sec;

   // Set RTC alarm time
   if (ioctl(rtc_fd, RTC_ALM_SET, &rtc_tm) < 0)
   {
      error("rtc ioctl RTC_ALM_SET error");
      return LIBWRAP_RET_ERR;
   }

   // Enable alarm interrupts
   if (ioctl(rtc_fd, RTC_AIE_ON, 0) < 0)
   {
      error("rtc ioctl RTC_ALE_ON error");
      return -1;
   }
*/
   return 0;
}

/** clear alarm callback */
int hal_rtc_remove_alarm(hal_rtc_alarm_t *alarm)
{
/*
   // disable alarm interrupts
   if (ioctl(rtc_fd, RTC_AIE_OFF, 0) < 0)
   {
      error("rtc ioctl RTC_ALE_OFF error");
      return LIBWRAP_RET_ERR;
   }
*/
   return 0;
}

static void *rtc_thread(void *arg)
{
   int res;
   fd_set rfds;
   struct timeval tv;
   unsigned long data;

   // Set wait tick timeout
   tv.tv_sec = CFG_RTC_THREAD_TICK_SEC;
   tv.tv_usec = 0;

   while(!terminate_thread)
   {
      FD_ZERO(&rfds);
      FD_SET(rtc_fd, &rfds);

      if ((res = select(rtc_fd+1, &rfds, NULL, NULL, &tv)) > 0)
      {
         if (read(rtc_fd, &data, sizeof(unsigned long)) < 0)
         {
            TRACE_ERROR("rtc read error");
            continue;
         }

         // TODO: send signal to parent RTC module
      }
      else if (res < 0)
      {
         TRACE_ERROR("select failed");
      }
   }
   
   return NULL;
}

