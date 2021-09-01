/**
 * \file main.c      \brief Hello test
 */

#include "system.h"

TRACE_TAG(Main);
#if !ENABLE_TRACE_MAIN
#undef TRACE
#define TRACE(...)
#endif


int main(void)
{
   // Initialize HW board
   VERIFY_FATAL(board_init() == 0);

   while(1)
   {     
    	TRACE("Alive");
		hal_delay_ms(1000);
   }

   return 0;
}
