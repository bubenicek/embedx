
#include "system.h"

TRACE_TAG(board);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

/** Low level board initialization */
int board_init(void)
{
   int res = 0;
   RCC_OscInitTypeDef RCC_OscInitStruct;
   RCC_ClkInitTypeDef RCC_ClkInitStruct;

   
   // Reset of all peripherals, Initializes the Flash interface and the Systick.
   HAL_Init();
     
   // System Clock Configuration
   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
   RCC_OscInitStruct.HSEState = RCC_HSE_ON;
   RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
   RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
   HAL_RCC_OscConfig(&RCC_OscInitStruct);

   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
   HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
   
   // GPIO Ports Clock Enable 
   __HAL_RCC_GPIOA_CLK_ENABLE();  
   __HAL_RCC_GPIOB_CLK_ENABLE();  
   __HAL_RCC_GPIOC_CLK_ENABLE();  

   // Initialize HAL drivers
   res += hal_time_init();
   res += hal_console_init();
   res += hal_gpio_init(); 

   TRACE("Board init, SYSCLK_Frequency: %d  HCLK_Frequency: %d", HAL_RCC_GetSysClockFreq(), HAL_RCC_GetHCLKFreq());

   return res;
}
