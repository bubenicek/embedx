
#include "system.h"

//TRACE_TAG(board);
//#if ! ENABLE_TRACE_HAL
//#undef TRACE
//#define TRACE(...)
//#endif

static void sigint_handler(int sig)
{  
#ifdef CFG_APP_SHUTDOWN_HANDLER
   CFG_APP_SHUTDOWN_HANDLER();
#endif // CFG_APP_SHUTDOWN_HANDLER   
   
   exit(0);
}

int board_init(void)
{
   int res = 0;
   
   // Initialize HAL drivers
   res += trace_init();
   res += hal_time_init();
   res += hal_console_init();
   res += hal_gpio_init();
   
   signal(SIGINT, sigint_handler);

   return res;
}

int board_deinit(void)
{
   return 0;
}
