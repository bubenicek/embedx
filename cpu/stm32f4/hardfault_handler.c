/* from: http://blog.frankvh.com/2011/12/07/cortex-m3-m4-hard-fault-handler/
 * for some more info see http://blog.feabhas.com/2013/02/developing-a-generic-hard-fault-handler-for-arm-cortex-m3cortex-m4/
 * -------
 *  From Joseph Yiu (Definitive Guide to the ARM Cortex M3), minor edits by FVH
 *  hard fault handler in C,
 *  with stack frame location as input parameter
 *  called from HardFault_Handler in file xxx.s
 */

#include "stm32f4xx.h"
#include "debug_uart_printf.h"


#ifdef _DEBUG

/* assembler section of Hard Fault handler code - prepare for core set registers dump */
void HardFault_Handler(void)
{
    __asm volatile (
                   "    TST lr, #4              \n"     // test for Main Stack Pointer or Process Stack Pointer
                   "    ITE EQ                  \n"
                   "    MRSEQ r0, MSP           \n"
                   "    MRSNE r0, PSP           \n"
                   "    B hard_fault_handler_c  \n"
   );
}


/* c section of Hard fault handler code - dump core set registers */
void hard_fault_handler_c(unsigned int * hardfault_args)
{
  unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;


  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);

  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);

  debug_uart_printf("\r\n\r\n[Hard fault handler ]\r\n");
  debug_uart_printf("R0  = 0x%08x\r\n", stacked_r0);
  debug_uart_printf("R1  = 0x%08x\r\n", stacked_r1);
  debug_uart_printf("R2  = 0x%08x\r\n", stacked_r2);
  debug_uart_printf("R3  = 0x%08x\r\n", stacked_r3);
  debug_uart_printf("R12 = 0x%08x\r\n", stacked_r12);
  debug_uart_printf("LR [R14] = 0x%08x  subroutine call return address\r\n", stacked_lr);
  debug_uart_printf("PC [R15] = 0x%08x  program counter\r\n", stacked_pc);
  debug_uart_printf("PSR   = 0x%08x\r\n", stacked_psr);
  debug_uart_printf("BFAR  = 0x%08x\r\n", SCB->BFAR);    // 0xE000ED38
  debug_uart_printf("CFSR  = 0x%08x\r\n", SCB->CFSR);    // 0xE000ED28
  debug_uart_printf("HFSR  = 0x%08x\r\n", SCB->HFSR);    // 0xE000ED2C
  debug_uart_printf("DFSR  = 0x%08x\r\n", SCB->DFSR);    // 0xE000ED30
  debug_uart_printf("AFSR  = 0x%08x\r\n", SCB->AFSR);    // 0xE000ED3C
  debug_uart_printf("SHCSR = 0x%08x\r\n", SCB->SHCSR);

  while (1);
}

#endif /* DEBUG */

