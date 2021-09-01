
#include "system.h"

#if ENABLE_TRACE_HAL
TRACE_TAG(hal_uuid);
#else
#include "trace_undef.h"
#endif

/** Get unique board ID */
uint32_t hal_get_board_uuid32(void)
{
   TRACE("%s", __FUNCTION__);
   return 0;
}

/** Generate uuid board string */
char *hal_get_board_uuid(char *uuid, int bufsize)
{
   TRACE("%s", __FUNCTION__);
   return uuid;;
} 
