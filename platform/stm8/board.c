
#include "system.h"

#define TRACE_TAG "board"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

int board_init(void)
{
   int res = 0;

   DISABLE_INTERRUPTS();
/*
   CLK_DeInit();
  
   // Configure the Fcpu to DIV1
   CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);

   // Configure the HSI prescaler to the optimal value 
   CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);

   // Configure the system clock to use HSE clock source and to run at 24Mhz 
   CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);
*/
   // Clock divider to HSI/1
   CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
     
   // Initialize HAL drivers
   res += hal_time_init();
   res += hal_gpio_init(); 

   // Global enable interrupts
   ENABLE_INTERRUPTS();

   return res;
}


#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */


  /* Infinite loop */
  while (1)
  {
  }
}
#endif
