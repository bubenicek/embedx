
#ifndef __TRACE_H
#define __TRACE_H

#if defined(CFG_DEBUG) && (CFG_DEBUG == 1)

#ifndef PSTR
#define PSTR(x) x
#endif

#define TRACE_NL  "\n"

#ifndef CFG_TRACE_HAS_FLOAT
#define CFG_TRACE_HAS_FLOAT            1
#endif

int trace_init(void);
void trace_printf(const char *fmt, ...);
const char *trace_uptime(void);
const char *trace_systime(void);
void trace_dump(const void *buffer, int buff_len);

#if defined(CFG_TRACE_TIMESTAMP_SYSTIME) && (CFG_TRACE_TIMESTAMP_SYSTIME == 1)
 #define trace_timestamp  trace_systime
#else
 #define trace_timestamp  trace_uptime
#endif

typedef struct
{
   const char *name;
   uint8_t enabled;

} trace_tag_t;

/** Tag name definition */
#define TRACE_TAG(_name) \
  trace_tag_t trace_tag_##_name = {.name = #_name, .enabled = 1}; \
  static const trace_tag_t *trace_tag = &trace_tag_##_name; 

/** Tag reference by name definition */
#define TRACE_GROUP(_name) \
  extern trace_tag_t trace_tag_##_name;   \
  static const trace_tag_t *trace_tag = &trace_tag_##_name;

/** Tag reference */
#define TRACE_TAG_NAME(_name) \
  extern trace_tag_t trace_tag_##_name;  

 #define TRACE_PRINTF(_format, ...) {              \
   if (trace_tag->enabled)                         \
      trace_printf(PSTR(_format), ## __VA_ARGS__); \
 }

#if CFG_CMSIS_OS_API
 #include "cmsis_os.h"
 #define TRACE(_format, ...) TRACE_PRINTF("%s %-10.10s     " _format TRACE_NL, trace_timestamp(), trace_tag->name, ## __VA_ARGS__)
 #define TRACE_PRINTFF(_format, ...) TRACE_PRINTF("%s %-10.10s     " _format, trace_timestamp(),  trace_tag->name, ## __VA_ARGS__)
 #define TRACE_ERROR(_format, ...) TRACE_PRINTF("%s %-10.10s *E  " _format "   %s:%d" TRACE_NL, trace_timestamp(), trace_tag->name, ## __VA_ARGS__, __FILE__, __LINE__)
#else
 #define TRACE(_format, ...) TRACE_PRINTF(PSTR("%s %-14.14s     " _format TRACE_NL), trace_timestamp(), trace_tag->name, ## __VA_ARGS__)
 #define TRACE_PRINTFF(_format, ...) TRACE_PRINTF(PSTR("%s %-14.14s     " _format), trace_timestamp(), trace_tag->name, ## __VA_ARGS__)
 #define TRACE_ERROR(_format, ...) TRACE_PRINTF(PSTR("%s %-14.14s *E  " _format "   %s:%d" TRACE_NL), trace_timestamp(), trace_tag->name, ## __VA_ARGS__, __FILE__, __LINE__)
#endif   // CFG_CMSIS_OS_API

#define TRACE_DUMP trace_dump

#ifndef ASSERT
   #define ASSERT(EX) {                                                             \
      if (!(EX))                                                                    \
      {                                                                             \
         TRACE_ERROR("(%s) Assert failed at %s:%d", #EX, __FUNCTION__, __LINE__);   \
         while(1) {hal_delay_ms(80); hal_led_toggle(LED_ERROR);}                    \
      }                                                                             \
   }
#endif   // ASSERT

#ifndef VERIFY
   #define VERIFY(EX) {                                                             \
      if (!(EX))                                                                    \
      {                                                                             \
         TRACE_ERROR("(%s) Verify failed at %s:%d", #EX, __FUNCTION__, __LINE__);   \
      }                                                                             \
   }
#endif   // VERIFY

#ifndef VERIFY_FATAL
   #define VERIFY_FATAL(EX) {                                                      \
      if (!(EX))                                                                   \
      {                                                                            \
         TRACE_ERROR("(%s) Fatal error at %s:%d", #EX, __FUNCTION__, __LINE__);    \
            while(1) {hal_delay_ms(80); hal_led_toggle(LED_ERROR);}                \
      }                                                                            \
   }
#endif   // VERIFY_FATAL

#else
 
 #define trace_init() 0

 #define TRACE_TAG(_name) 
 #define TRACE(...)
 #define TRACE_PRINTFF(...)
 #define TRACE_PRINTF(...)
 #define TRACE_ERROR(...)
 #ifndef ASSERT
 #define ASSERT(...)
 #endif
 #define VERIFY(EX)  (EX)
 #define TRACE_DUMP

 #define VERIFY_FATAL(EX) {                                                 \
   if (!(EX))                                                               \
   {                                                                        \
      while(1) {hal_delay_ms(80);  hal_led_toggle(LED_ERROR);}              \
   }                                                                        \
 }

#endif   // CFG_DEBUG

#endif   // __TRACE_H

