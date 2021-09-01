

#ifndef __STM8S_IT_H
#define __STM8S_IT_H

// Interrupts declarations
INTERRUPT_HANDLER(UART1_RX_IRQHandler, 18);
INTERRUPT_HANDLER(EXTI_PORTB_IRQHandler, 4);
INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23);

#endif // __STM8S_IT_H

