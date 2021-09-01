#include <pj/os.h>
#include <pj/errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/time.h>
#include <errno.h>
#include <clock.h>
#include <debug.h>

#define USEC_PER_SEC	1000000

PJ_DEF(pj_status_t) pj_get_timestamp(pj_timestamp *ts)
{
/*
   pj_time_val tv;
   pj_gettimeofday(&tv);
   ts->u64 = tv.sec;
   ts->u64 *= USEC_PER_SEC;
   ts->u64 += tv.msec;
*/

   ts->u32.hi = 0;
   ts->u32.lo = clock_time();

   return PJ_SUCCESS;
}

PJ_DEF(pj_status_t) pj_get_timestamp_freq(pj_timestamp *freq)
{
   freq->u32.hi = 0;
   freq->u32.lo = USEC_PER_SEC;
   return PJ_SUCCESS;
}


