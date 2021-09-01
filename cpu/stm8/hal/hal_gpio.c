
#include "system.h"

#define TRACE_TAG "hal-gpio"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

typedef struct
{
   __IO GPIO_TypeDef *port;
   GPIO_Pin_TypeDef pin;
   GPIO_Mode_TypeDef mode;
   uint8_t init_output_state;

} hal_gpio_def_t;


static hal_gpio_def_t gpio_def[] = {CFG_HAL_GPIO_DEF};
#define NUM_GPIO   (sizeof(gpio_def) / sizeof(hal_gpio_def_t))

static hal_gpio_irq_handler_t irq_handler = NULL;
static uint16_t inputs_state = 0;

int hal_gpio_init(void)
{
   int ix;
   int input_index = 0;
   
   inputs_state = 0;

   // Initialize the Interrupt sensitivity 
   EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_RISE_FALL);
   EXTI_SetTLISensitivity(EXTI_SENSITIVITY_RISE_FALL);
   
   for (ix = 0; ix < NUM_GPIO; ix++)
   {
      GPIO_Init(gpio_def[ix].port, gpio_def[ix].pin, gpio_def[ix].mode);
      
      if (gpio_def[ix].mode == GPIO_MODE_OUT_PP_LOW_FAST)
      {
        hal_gpio_set(ix, gpio_def[ix].init_output_state); 
      }
      else if (gpio_def[ix].mode == GPIO_MODE_IN_FL_IT)
      {
         if (hal_gpio_get(ix))
            inputs_state |= (1 << input_index);
            
         input_index++;
      }
   }
   
   return 0;
}

void hal_gpio_set(hal_gpio_t gpio, uint8_t state)
{
   ASSERT(gpio < NUM_GPIO);

   if (state)
      GPIO_WriteHigh(gpio_def[gpio].port, gpio_def[gpio].pin);
   else
      GPIO_WriteLow(gpio_def[gpio].port, gpio_def[gpio].pin);
}

void hal_gpio_toggle(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);  
   GPIO_WriteReverse(gpio_def[gpio].port, gpio_def[gpio].pin);
}

uint8_t hal_gpio_get(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   return GPIO_ReadInputPin(gpio_def[gpio].port, gpio_def[gpio].pin);
}

int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler)
{
   UNUSED(gpio);
   UNUSED(edge);
   
   DISABLE_INTERRUPTS();
   
   // TODO:
   irq_handler = handler;
   
   ENABLE_INTERRUPTS();

   return 0;
}

INTERRUPT_HANDLER(EXTI_PORTB_IRQHandler, 4)
{
   // TODO:
   if (irq_handler != NULL)
   {
      irq_handler(GPIO_INPUT1);
   }
}
