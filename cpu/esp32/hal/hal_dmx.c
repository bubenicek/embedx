
#include "system.h"

TRACE_TAG(hal_dmx);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

int hal_dmx_init(hal_dmx_t dmx, uint32_t dmx_break_delay)
{
   return 0;
}

int hal_dmx_deinit(hal_dmx_t dmx)
{
   return 0;
}

int hal_dmx_write(hal_dmx_t dmx, uint8_t *buf, int count)
{  
   return 0;
}
