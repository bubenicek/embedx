
#ifndef __HAL_DEF_H
#define __HAL_DEF_H

#include "stm32f1xx_hal.h"

typedef struct
{
   GPIO_TypeDef *port;
   uint32_t pin;
   uint32_t mode;
   uint8_t  pull;
   uint8_t  init_output_state;

} hal_gpio_def_t;


//
// UART definition
//

/** UART GPIO configuration */
typedef struct
{
   GPIO_TypeDef *port;
   uint32_t pin;
   uint32_t gpio_alternate;

} hal_uart_gpio_t;


/** UART definition */
typedef struct
{
   USART_TypeDef *usart;
   uint32_t baudrate;
   hal_uart_gpio_t tx_pin;
   hal_uart_gpio_t rx_pin;
   uint32_t irqn;

} hal_uart_def_t;


//
// CAN definition
//

#ifndef CFG_HAL_CAN_NUM_FILTERS
#define CFG_HAL_CAN_NUM_FILTERS     1
#endif

/** CAN GPIO configuration */
typedef struct
{
   GPIO_TypeDef *port;
   uint32_t pin;

} hal_can_gpio_t;

typedef struct
{
   uint32_t id;
   uint32_t mask;

} hal_can_filter_t;

typedef struct
{
   CAN_TypeDef *instance;

   uint32_t prescaler;
   uint32_t sync_jump_width;
   uint32_t time_seg1;
   uint32_t time_seg2;

   hal_can_filter_t filter[CFG_HAL_CAN_NUM_FILTERS];

   hal_can_gpio_t gpio_tx;
   hal_can_gpio_t gpio_rx;

   bool remap_can1_2;

} hal_can_def_t;



//
// Timer
//
#include "hal_timer.h"

typedef struct
{
   // Definition
   TIM_TypeDef  *instance;
   uint32_t irqn;

   // Runtime
   TIM_HandleTypeDef base;
   hal_timer_t id;
   hal_timer_type_t type;
   hal_timer_cb_t cb;
   void *arg; 
   bool running;

} hal_timer_def_t;


#endif   // __HAL_DEF_H