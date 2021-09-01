
#include <string.h>
#include "system.h"

TRACE_TAG(hal_gpio);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

static const hal_gpio_def_t gpio_def[] = CFG_HAL_GPIO_DEF;
#define NUM_GPIO   (sizeof(gpio_def) / sizeof(hal_gpio_def_t))

static hal_gpio_irq_handler_t gpio_irq_handler = NULL;


int hal_gpio_init(void)
{
   int ix;
   GPIO_InitTypeDef  GPIO_InitStructure;

   for (ix = 0; ix < NUM_GPIO; ix++)
   {
      GPIO_InitStructure.Pull  = gpio_def[ix].pull;
      GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
      GPIO_InitStructure.Mode  = gpio_def[ix].mode;
      GPIO_InitStructure.Pin = gpio_def[ix].pin;

      if (gpio_def[ix].mode == GPIO_MODE_OUTPUT_PP)
         HAL_GPIO_WritePin(gpio_def[ix].port, gpio_def[ix].pin, gpio_def[ix].init_output_state);

      HAL_GPIO_Init(gpio_def[ix].port, &GPIO_InitStructure);
   }

   return 0;
}

/** Configure GPIO as input or output */
int hal_gpio_configure(hal_gpio_t gpio, hal_gpio_mode_t mode)
{
   GPIO_InitTypeDef  GPIO_InitStructure;

   ASSERT(gpio < NUM_GPIO);
   
   GPIO_InitStructure.Pull  = gpio_def[gpio].pull;
   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
   GPIO_InitStructure.Mode  = (mode == HAL_GPIO_MODE_OUTPUT) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
   GPIO_InitStructure.Pin = gpio_def[gpio].pin;

   HAL_GPIO_Init(gpio_def[gpio].port, &GPIO_InitStructure);   
   
   return 0;
}

void hal_gpio_set(hal_gpio_t gpio, uint8_t state)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == GPIO_MODE_OUTPUT_PP);
   HAL_GPIO_WritePin(gpio_def[gpio].port, gpio_def[gpio].pin, state);
}

void hal_gpio_toggle(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == GPIO_MODE_OUTPUT_PP);
   HAL_GPIO_TogglePin(gpio_def[gpio].port, gpio_def[gpio].pin);
}

uint8_t hal_gpio_get(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   return HAL_GPIO_ReadPin(gpio_def[gpio].port, gpio_def[gpio].pin);
}

int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler)
{
   gpio_irq_handler = handler;

   HAL_NVIC_SetPriority(EXTI4_IRQn, CFG_HAL_GPIO_PRIORITY, 0);
   HAL_NVIC_EnableIRQ(EXTI4_IRQn);

   return 0;
}

void EXTI4_IRQHandler(void)
{
   // TODO: zobecnit !!
   if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
   {
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);

      if (gpio_irq_handler != NULL)
         gpio_irq_handler(0);
   }
}
