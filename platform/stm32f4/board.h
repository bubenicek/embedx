
#ifndef __STM32_HLUK_BOARD_H
#define __STM32_HLUK_BOARD_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "stm32f4xx_conf.h"
#include "compiler.h"

#include "sys/posix/dirent.h"

#include "FreeRTOSConfig.h"

//----------------------------------------------------------------------------
//                         Platform Configuration
//----------------------------------------------------------------------------

#define CFG_HW_VERSION_MODEL            0
#define CFG_HW_VERSION_REVISION         1

#define CFG_HAL_HAS_RTC                 1
#define CFG_HAL_WDG_ENABLED             0
#define CFG_HAL_CONSOLE_BUFFERED        1
#define CFG_HAL_UART_ENABLED            1
#define CFG_HAL_SPI_USE_DMA             1

//-----------------------------------------------------------------------------
// 										GPIO
//-----------------------------------------------------------------------------

#define CFG_HAL_GPIO_IRQ_PRIORITY               configLIBRARY_KERNEL_INTERRUPT_PRIORITY

#define CFG_HAL_GPIO_DEF { \
/* 0 */    {GPIOD, GPIO_Pin_10, GPIO_Mode_OUT, 0},     /* LED0 */          \
/* 1 */    {GPIOB, GPIO_Pin_15, GPIO_Mode_OUT, 0},     /* LED1 */          \
/* 2 */    {GPIOB, GPIO_Pin_10, GPIO_Mode_OUT, 0},     /* LED2 */          \
/* 3 */    {GPIOC, GPIO_Pin_7,  GPIO_Mode_OUT, 1},     /* ADC_CS */        \
/* 4 */    {GPIOC, GPIO_Pin_8,  GPIO_Mode_IN},         /* ADC_BUSY */      \
/* 5 */    {GPIOA, GPIO_Pin_11, GPIO_Mode_OUT, 1},     /* ADC_CNV */       \
/* 6 */    {GPIOA, GPIO_Pin_12, GPIO_Mode_OUT, 1},     /* ADC_PD */        \
/* 7 */    {GPIOI, GPIO_Pin_9,  GPIO_Mode_OUT, 0},     /* RELE0_coFUL */   \
/* 8 */    {GPIOI, GPIO_Pin_11, GPIO_Mode_OUT, 0},     /* RELE0_coEMP */   \
/* 9 */    {GPIOI, GPIO_Pin_10, GPIO_Mode_IN},         /* RELE0_reFUL */   \
/* 10 */   {GPIOE, GPIO_Pin_4,  GPIO_Mode_OUT, 0},     /* RELE1_coFUL */   \
/* 11 */   {GPIOE, GPIO_Pin_6,  GPIO_Mode_OUT, 0},     /* RELE1_coEMP */   \
/* 12 */   {GPIOE, GPIO_Pin_5,  GPIO_Mode_IN},         /* RELE1_reFUL */   \
/* 13 */   {GPIOC, GPIO_Pin_13, GPIO_Mode_OUT, 0},     /* RELE2_coFUL */   \
/* 14 */   {GPIOI, GPIO_Pin_8,  GPIO_Mode_OUT, 0},     /* RELE2_coEMP */   \
/* 15 */   {GPIOE, GPIO_Pin_3,  GPIO_Mode_IN},         /* RELE2_reFUL */   \
/* 16 */   {GPIOA, GPIO_Pin_3,  GPIO_Mode_OUT, 0},     /* LD_SAT1 */       \
/* 17 */   {GPIOH, GPIO_Pin_5,  GPIO_Mode_OUT, 0},     /* LD_SAT2 */       \
/* 18 */   {GPIOH, GPIO_Pin_4,  GPIO_Mode_OUT, 0},     /* LD_SAT3 */       \
/* 19 */   {GPIOH, GPIO_Pin_3,  GPIO_Mode_OUT, 0},     /* LD_SAT4 */       \
/* 20 */   {GPIOC, GPIO_Pin_10, GPIO_Mode_OUT, 0},     /* LD_5V_NEG */     \
/* 21 */   {GPIOC, GPIO_Pin_11, GPIO_Mode_OUT, 0},     /* LD_5V_POS */     \
/* 22 */   {GPIOC, GPIO_Pin_12, GPIO_Mode_OUT, 0},     /* LD_15V_NEG */    \
/* 23 */   {GPIOD, GPIO_Pin_0,  GPIO_Mode_OUT, 0},     /* LD_15V_POS */    \
/* 24 */   {GPIOD, GPIO_Pin_1,  GPIO_Mode_OUT, 0},     /* LD_25V_POS */    \
/* 25 */   {GPIOF, GPIO_Pin_5,  GPIO_Mode_OUT, 0},     /* PWR_S0 */        \
/* 26 */   {GPIOF, GPIO_Pin_4,  GPIO_Mode_OUT, 0},     /* PWR_S1 */        \
/* 27 */   {GPIOF, GPIO_Pin_2,  GPIO_Mode_OUT, 0},     /* PWR_S2 */        \
/* 28 */   {GPIOG, GPIO_Pin_7,  GPIO_Mode_IN, 0, EXTI_Line7, EXTI_PortSourceGPIOG, EXTI_PinSource7, EXTI9_5_IRQn},         /* UN_LO_AIN0 */    \
/* 29 */   {GPIOG, GPIO_Pin_6,  GPIO_Mode_IN, 0, EXTI_Line6, EXTI_PortSourceGPIOG, EXTI_PinSource6, EXTI9_5_IRQn},         /* UP_HI_AIN0 */    \
/* 30 */   {GPIOG, GPIO_Pin_8,  GPIO_Mode_IN, 0, EXTI_Line8, EXTI_PortSourceGPIOG, EXTI_PinSource8, EXTI9_5_IRQn},         /* UN_LO_AIN1 */    \
/* 31 */   {GPIOC, GPIO_Pin_6,  GPIO_Mode_IN, 0, EXTI_Line6, EXTI_PortSourceGPIOC, EXTI_PinSource6, EXTI9_5_IRQn},         /* UP_HI_AIN1 */    \
/* 32 */   {GPIOG, GPIO_Pin_3,  GPIO_Mode_IN, 0, EXTI_Line3, EXTI_PortSourceGPIOG, EXTI_PinSource3, EXTI3_IRQn},           /* UN_LO_AIN2 */    \
/* 33 */   {GPIOG, GPIO_Pin_2,  GPIO_Mode_IN, 0, EXTI_Line2, EXTI_PortSourceGPIOG, EXTI_PinSource2, EXTI2_IRQn},           /* UP_HI_AIN2 */    \
/* 34 */   {GPIOG, GPIO_Pin_5,  GPIO_Mode_IN, 0, EXTI_Line5, EXTI_PortSourceGPIOG, EXTI_PinSource5, EXTI9_5_IRQn},         /* UN_LO_AIN3 */    \
/* 35 */   {GPIOG, GPIO_Pin_4,  GPIO_Mode_IN, 0, EXTI_Line4, EXTI_PortSourceGPIOG, EXTI_PinSource4, EXTI4_IRQn},           /* UP_HI_AIN3 */    \
}

 uint32_t exti_line;
   uint32_t exti_port_source;
   uint32_t exti_pin_source;
   uint8_t  exti_irqn;



//-----------------------------------------------------------------------------
// 										   LED
//-----------------------------------------------------------------------------

#define CFG_HAL_LED_DEF { \
/* 0 */   {HAL_GPIO0,   1},      \
/* 1 */   {HAL_GPIO1,   1},      \
/* 2 */   {HAL_GPIO2,   1},      \
/* 3 */   {HAL_GPIO16,  1},  /* LD_SAT... */  \
/* 4 */   {HAL_GPIO17,  1},      \
/* 5 */   {HAL_GPIO18,  1},      \
/* 6 */   {HAL_GPIO19,  1},      \
/* 7 */   {HAL_GPIO20,  1}, /* LD_voltage */     \
/* 8 */   {HAL_GPIO21,  1},      \
/* 9 */   {HAL_GPIO22,  1},      \
/* 10 */  {HAL_GPIO23,  1},      \
/* 11 */  {HAL_GPIO24,  1},      \
}

//-----------------------------------------------------------------------------
// 										   SPI
//-----------------------------------------------------------------------------

#define CFG_HAL_SPI_DEF { \
   {SPI2, SPI_BaudRatePrescaler_2, RCC_PERIPH_APB1, RCC_APB1Periph_SPI2, {{GPIO_Pin_14, GPIOB, GPIO_AF_SPI2}, {GPIO_Pin_3, GPIOI, GPIO_AF_SPI2}, {GPIO_Pin_1, GPIOI, GPIO_AF_SPI2}}},  \
}

//-----------------------------------------------------------------------------
// 										   UART
//-----------------------------------------------------------------------------

#define CFG_HAL_UART_IRQ_PRIORITY              configLIBRARY_KERNEL_INTERRUPT_PRIORITY

#define CFG_HAL_UART_DEF {\
   {USART1, CONSOLE_UART_BAUDRATE, RCC_PERIPH_APB2, RCC_APB2Periph_USART1, USART1_IRQn, {{GPIO_Pin_9, GPIOA, GPIO_AF_USART1}, {GPIO_Pin_10, GPIOA, GPIO_AF_USART1}}}, \
}

//-----------------------------------------------------------------------------
// 										   ADC
//-----------------------------------------------------------------------------

#define CFG_HAL_ADC_DEF { \
   {ADC3, TM_ADC_Channel_9}, \
}


//-----------------------------------------------------------------------------
// 										   DMA
//-----------------------------------------------------------------------------

/* DMA1 preemption priority */
#define DMA1_NVIC_PREEMPTION_PRIORITY        configLIBRARY_KERNEL_INTERRUPT_PRIORITY

/* DMA2 preemption priority */
#define DMA2_NVIC_PREEMPTION_PRIORITY        configLIBRARY_KERNEL_INTERRUPT_PRIORITY


//-----------------------------------------------------------------------------
//                               Board peripherials
//-----------------------------------------------------------------------------

//#define DEBUG_GPIO                     HAL_GPIO2

// LED
#define LED_SYSTEM                     HAL_LED2
#define LED_ERROR                      HAL_LED0
#define LED_ACTIVITY                   HAL_LED1

#define LED_SAT1                       HAL_LED3
#define LED_SAT2                       HAL_LED4
#define LED_SAT3                       HAL_LED5
#define LED_SAT4                       HAL_LED6

#define LED_5V_NEG                     HAL_LED7
#define LED_5V_POS                     HAL_LED8
#define LED_15V_NEG                    HAL_LED9
#define LED_15V_POS                    HAL_LED10
#define LED_25V_POS                    HAL_LED11


// Console UART
#define CONSOLE_UART                   HAL_UART0
#define CONSOLE_UART_BAUDRATE          115200


// ADC
#define LTC2357_SPI                    HAL_SPI0
#define LTC2357_GPIO_CS                HAL_GPIO3
#define LTC2357_GPIO_BUSY              HAL_GPIO4
#define LTC2357_GPIO_CNV               HAL_GPIO5
#define LTC2357_GPIO_PD                HAL_GPIO6


// Relays
#define RELE0_GPIO_COFULL              HAL_GPIO7
#define RELE0_GPIO_COEMP               HAL_GPIO8
#define RELE0_GPIO_REFULL              HAL_GPIO9

#define RELE1_GPIO_COFULL              HAL_GPIO10
#define RELE1_GPIO_COEMP               HAL_GPIO11
#define RELE1_GPIO_REFULL              HAL_GPIO12

#define RELE2_GPIO_COFULL              HAL_GPIO13
#define RELE2_GPIO_COEMP               HAL_GPIO14
#define RELE2_GPIO_REFULL              HAL_GPIO15

// PWR ADC
#define PWR_ADC                        HAL_ADC0
#define PWR_S0                         HAL_GPIO25
#define PWR_S1                         HAL_GPIO26
#define PWR_S2                         HAL_GPIO27
#define PWR_NUM_CHANNELS               8


// ADC comparators
#define UN_LO_AIN0                     HAL_GPIO28
#define UP_HI_AIN0                     HAL_GPIO29

#define UN_LO_AIN1                     HAL_GPIO30
#define UP_HI_AIN1                     HAL_GPIO31

#define UN_LO_AIN2                     HAL_GPIO32
#define UP_HI_AIN2                     HAL_GPIO33

#define UN_LO_AIN3                     HAL_GPIO34
#define UP_HI_AIN3                     HAL_GPIO35


#endif   // __STM32_HLUK_BOARD_H
