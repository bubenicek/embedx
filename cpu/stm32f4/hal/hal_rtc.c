
#include "system.h"
#include <time.h>

//
// RTC oscilator types
//
#define RTC_OSC_TYPE_EXTERNAL       0
#define RTC_OSC_TYPE_LSI            1
#define RTC_OSC_TYPE_HSE            2

#define RTC_OSC_TYPE_EXT_SYNCH_PREDIV   0xFF
#define RTC_OSC_TYPE_LSI_SYNCH_PREDIV   0xFF
#define RTC_OSC_TYPE_HSE_SYNCH_PREDIV   7999

#define RTC_OSC_TYPE                RTC_OSC_TYPE_HSE
#define RTC_OSC_TYPE_SYNCH_PREDIV   RTC_OSC_TYPE_HSE_SYNCH_PREDIV


/** Init RTC */
int hal_rtc_init(void)
{
   RTC_InitTypeDef   RTC_InitStructure;
   EXTI_InitTypeDef EXTI_InitStructure;
   //NVIC_InitTypeDef NVIC_InitStructure;

   // Enable the PWR clock
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

   // Allow access to RTC
   PWR_BackupAccessCmd(ENABLE);

#if RTC_OSC_TYPE == RTC_OSC_TYPE_EXTERNAL
   // Enable the LSE OSC - external oscilator
   RCC_LSEConfig(RCC_LSE_ON);
   // Wait till LSE is ready
   while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

   // Select the RTC Clock Source
   RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

   // Configure the RTC prescaler
   RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
   RTC_InitStructure.RTC_SynchPrediv  = RTC_OSC_TYPE_HSE_SYNCH_PREDIV;
#elif RTC_OSC_TYPE == RTC_OSC_TYPE_LSI
   // LSI used as RTC source clock
   // The RTC Clock may varies due to LSI frequency dispersion.
   // Enable the LSI OSC
   RCC_LSICmd(ENABLE);

   // Wait till LSI is ready
   while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

   // Select the RTC Clock Source
   RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

   // Configure the RTC prescaler
   RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
   RTC_InitStructure.RTC_SynchPrediv  = RTC_OSC_TYPE_LSI_SYNCH_PREDIV;
#elif RTC_OSC_TYPE == RTC_OSC_TYPE_HSE
   // Enable the HSE OSC - external oscilator 8MHz
   RCC_HSEConfig(RCC_HSE_ON);
   // Wait till HSE is ready
   while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

   // Select the RTC Clock Source (1MHz)
   RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div8);

   // Configure the RTC prescaler (1Hz)
   RTC_InitStructure.RTC_AsynchPrediv = 124;
   RTC_InitStructure.RTC_SynchPrediv  = RTC_OSC_TYPE_HSE_SYNCH_PREDIV;
#else
#error "Not defined RTC_OSC_TYPE !"
#endif

   // Enable the RTC Clock
   RCC_RTCCLKCmd(ENABLE);

   // Wait for RTC APB registers synchronisation
   RTC_WaitForSynchro();

   // Configure the RTC data register and RTC prescaler
   RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
   RTC_Init(&RTC_InitStructure);

   // EXTI configuration
   //EXTI_ClearITPendingBit(EXTI_Line17);
   //EXTI_InitStructure.EXTI_Line = EXTI_Line17;
   //EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   // Enable the RTC Alarm Interrupt
   //NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
   //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LN_CFG_IRQ_PRIORITY_RTC;
   //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   //NVIC_Init(&NVIC_InitStructure);

   // Enable AlarmA interrupt
   //RTC_ITConfig(RTC_IT_ALRA, ENABLE);

   return 0;
}

/** Set RTC time in seconds from epoch 1970-01-01 00:00:00 +0000 (UTC) */
int hal_rtc_set_time(time_t time)
{
   RTC_TimeTypeDef RTC_TimeStruct;
   RTC_DateTypeDef RTC_DateStruct;
   struct tm *tm;

   tm = localtime(&time);

   // Set the RTC time
   RTC_TimeStruct.RTC_H12     =  RTC_H12_PM;
   RTC_TimeStruct.RTC_Hours   = tm->tm_hour;
   RTC_TimeStruct.RTC_Minutes = tm->tm_min;
   RTC_TimeStruct.RTC_Seconds = tm->tm_sec;
   RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);

   // set the RTC date
   RTC_DateStruct.RTC_Date = tm->tm_mday;
   RTC_DateStruct.RTC_Month = tm->tm_mon + 1;
   RTC_DateStruct.RTC_Year = tm->tm_year - 70;
   RTC_DateStruct.RTC_WeekDay = tm->tm_wday + 1; //rtcDow(2000 + RTC_DateStruct.RTC_Year, RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Date);
   RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);

   return 0;
}

/** Get RTC time in seconds from epoch 1970-01-01 00:00:00 +0000 (UTC). */
int hal_rtc_get_time(time_t *time)
{
   RTC_TimeTypeDef RTC_TimeStruct;
   RTC_DateTypeDef RTC_DateStruct;
   struct tm tm;

   //RTC_SubSeconds = RTC_GetSubSecond();
   RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
   RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

   tm.tm_mday = RTC_DateStruct.RTC_Date;
   tm.tm_mon = RTC_DateStruct.RTC_Month - 1;
   tm.tm_year = RTC_DateStruct.RTC_Year + 70;
   tm.tm_hour = RTC_TimeStruct.RTC_Hours;
   tm.tm_min = RTC_TimeStruct.RTC_Minutes;
   tm.tm_sec = RTC_TimeStruct.RTC_Seconds;
   tm.tm_wday = RTC_DateStruct.RTC_WeekDay - 1;

   *time = mktime(&tm);

   return 0;
}
