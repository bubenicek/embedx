
#ifndef __ARDUINO_BOARD_H
#define __ARDUINO_BOARD_H

#include <stdio.h>
#include "stm8s.h"


#define ENABLE_INTERRUPTS()       enableInterrupts()
#define DISABLE_INTERRUPTS()      disableInterrupts()

//
// GPIO configuration
//

#define CFG_HAL_GPIO_DEF  \
   {GPIOC, GPIO_PIN_1, GPIO_MODE_OUT_PP_LOW_FAST, 1},  /* Relay1 */   \
   {GPIOC, GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_FAST, 1},  /* Relay2 */   \
   {GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_FAST, 1},  /* Relay3 */   \
   {GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST, 1},  /* Relay4 */   \
   {GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_LOW_FAST, 1},  /* Relay5 */   \
   {GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST, 1},  /* Relay6 */   \
   {GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST, 1},  /* Relay7 */   \
   {GPIOC, GPIO_PIN_7, GPIO_MODE_OUT_PP_LOW_FAST, 1},  /* Relay8 */   \
   {GPIOB, GPIO_PIN_0, GPIO_MODE_IN_FL_IT},            /* Input1 */   \
   {GPIOB, GPIO_PIN_1, GPIO_MODE_IN_FL_IT},            /* Input2 */   \
   {GPIOB, GPIO_PIN_2, GPIO_MODE_IN_FL_IT},            /* Input3 */   \
   {GPIOB, GPIO_PIN_3, GPIO_MODE_IN_FL_IT},            /* Input4 */   \
   {GPIOB, GPIO_PIN_4, GPIO_MODE_IN_FL_IT},            /* Input5 */   \
   {GPIOB, GPIO_PIN_5, GPIO_MODE_IN_FL_IT},            /* Input6 */   \
   {GPIOB, GPIO_PIN_6, GPIO_MODE_IN_FL_IT},            /* Input7 */   \
   {GPIOB, GPIO_PIN_7, GPIO_MODE_IN_FL_IT},            /* Input8 */   \
   {GPIOE, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST, 0},  /* LED */      \
   {GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST, 0},  /* RS485 dir */\
   {GPIOF, GPIO_PIN_4, GPIO_MODE_IN_FL_IT},            /* Button */   \

  

//
// LED configuration
//
#define CFG_HAL_LED_DEF    \
   {HAL_GPIO16, 1},                                 /* LED system */ 
    
//
// Board configuration
//

#define CFG_HAL_NUM_UARTS                    1           // Number of uarts
#define CFG_HAL_UART_RS485_AFTER_TX_DELAY    5

#define LED_SYSTEM                           HAL_LED0
#define LED_ERROR                            HAL_LED0
#define GPIO_RS485_DIR                       HAL_GPIO17

// Outputs
#define GPIO_OUTPUT1                         HAL_GPIO0

// Inputs
#define GPIO_INPUT1                          HAL_GPIO8
#define GPIO_INPUT2                          GPIO_INPUT1+1
#define GPIO_INPUT3                          GPIO_INPUT1+2
#define GPIO_INPUT4                          GPIO_INPUT1+3
#define GPIO_INPUT5                          GPIO_INPUT1+4
#define GPIO_INPUT6                          GPIO_INPUT1+5
#define GPIO_INPUT7                          GPIO_INPUT1+6
#define GPIO_INPUT8                          GPIO_INPUT1+7

#endif   
