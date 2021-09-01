#include <pj/os.h>
#include <pj/errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/time.h>
#include <errno.h>

#define USEC_PER_SEC	1000000

extern unsigned int voip_clock_time(void);

PJ_DEF(pj_status_t) pj_get_timestamp(pj_timestamp *ts)
{
   ts->u32.hi = 0;
   ts->u32.lo = voip_clock_time();

   return PJ_SUCCESS;
}

PJ_DEF(pj_status_t) pj_get_timestamp_freq(pj_timestamp *freq)
{
   freq->u32.hi = 0;
   freq->u32.lo = USEC_PER_SEC;
   return PJ_SUCCESS;
}


