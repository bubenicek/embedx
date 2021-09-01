
#include "system.h"

#define TRACE_TAG "board"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

int board_init(void)
{
   int res = 0;
   
   // Disable interrupts
   DISABLE_INTERRUPTS();  
   
   // Initialize HAL drivers
   res += hal_time_init();
   res += hal_console_init();
   res += hal_gpio_init(); 

   // Global enable interrupts
   ENABLE_INTERRUPTS();

   TRACE("Arduino board init");   

   return res;
}
