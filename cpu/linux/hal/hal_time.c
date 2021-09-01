
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "system.h"

//TRACE_TAG(hal_time);
//#if ! ENABLE_TRACE_HAL
//#include "trace_undef.h"
//#endif

int hal_time_init(void)
{
  return 0;
}

hal_time_t hal_time_ms(void)
{
    static hal_time_t start_tm = 0;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    if (start_tm == 0)
    {
        start_tm = 1000 * tp.tv_sec + tp.tv_nsec / 1000000;
    }

    return (1000 * tp.tv_sec + tp.tv_nsec / 1000000) - start_tm;
}

void hal_delay_ms(hal_time_t ms)
{
   usleep(ms * 1000);
}

void hal_delay_us(hal_time_t us)
{
   usleep(us);
}


