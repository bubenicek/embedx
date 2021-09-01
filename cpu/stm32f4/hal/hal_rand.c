
#include "system.h"
#include "stm32f4xx_rng.h"

int hal_rand_init(void)
{
   // Enable RNG clock source 
   RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
   
   // Enable random generator
   RNG_Cmd(ENABLE);
   
   return 0;
}

uint32_t hal_rand_number(void)
{
   // Wait until random number is ready
   while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);

   // Get the random number
   return RNG_GetRandomNumber(); 
}
