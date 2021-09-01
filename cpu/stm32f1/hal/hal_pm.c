
#include "system.h"

void hal_reset(void)
{
   NVIC_SystemReset();   
}