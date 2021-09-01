
#include "system.h"

TRACE_TAG(board);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif


int board_init(void)
{
   RCC_ClocksTypeDef RCC_Clocks;

   // Ensure all priority bits are assigned as preemption priority bits.
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

   // GPIO clock enable
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|
                          RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG|RCC_AHB1Periph_GPIOH|RCC_AHB1Periph_GPIOI, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   // Initialize HAL drivers
   hal_wdg_init();
   hal_time_init();
   hal_console_init();
   hal_gpio_init();
   //hal_rtc_init();
   //hal_rand_init();
   trace_init();

   RCC_GetClocksFreq(&RCC_Clocks);

   TRACE("Board init");
   TRACE("  SYSCLK_Frequency: %d", RCC_Clocks.SYSCLK_Frequency);
   TRACE("  HCLK_Frequency: %d", RCC_Clocks.HCLK_Frequency);
   TRACE("  PCLK1: %d", RCC_Clocks.PCLK1_Frequency);
   TRACE("  PCLK2: %d", RCC_Clocks.PCLK2_Frequency);

/*
   TRACE("FreeRTOS IRQ priority:");
   TRACE("   configPRIO_BITS=%d", configPRIO_BITS);
   TRACE("   configLIBRARY_LOWEST_INTERRUPT_PRIORITY=%d", configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
   TRACE("   configLIBRARY_KERNEL_INTERRUPT_PRIORITY=%d", configLIBRARY_KERNEL_INTERRUPT_PRIORITY);
   TRACE("   configKERNEL_INTERRUPT_PRIORITY=%d", configKERNEL_INTERRUPT_PRIORITY);
   TRACE("   configMAX_SYSCALL_INTERRUPT_PRIORITY=%d", configMAX_SYSCALL_INTERRUPT_PRIORITY);
*/

   return 0;
}

/** Deinitialize board */
int board_deinit(void)
{
   __set_PRIMASK(1);
   RCC_DeInit();
   SysTick->CTRL = 0;
   SysTick->LOAD = 0;
   SysTick->VAL = 0;
   RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

   return 0;
}

