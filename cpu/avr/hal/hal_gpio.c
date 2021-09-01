
#include "system.h"

#define TRACE_TAG "hal-gpio"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

typedef struct
{
   uint8_t port;
   uint8_t pin;
   uint8_t mode;
   uint8_t  init_output_state;

} hal_gpio_def_t;


static const hal_gpio_def_t gpio_def[] = {CFG_HAL_GPIO_DEF};
#define NUM_GPIO   (sizeof(gpio_def) / sizeof(hal_gpio_def_t))

static hal_gpio_irq_handler_t btn_irq_handler = NULL;


int hal_gpio_init(void)
{
   int ix;

   for (ix = 0; ix < NUM_GPIO; ix++)
   {
      if (gpio_def[ix].mode == GPIO_MODE_OUTPUT)
      {
         switch(gpio_def[ix].port)
         {
            case GPIO_PORTB:
               DDRB |= (1 << gpio_def[ix].pin);
               break;
            case GPIO_PORTC:
               DDRC |= (1 << gpio_def[ix].pin);
               break;
            case GPIO_PORTD:
               DDRD |= (1 << gpio_def[ix].pin);
               break;
            default:
               return -1;
         }
         
         hal_gpio_set(ix, gpio_def[ix].init_output_state);
      }
      else
      {
         // TODO: Input
      }
   }
   
   return 0;
}

void hal_gpio_set(hal_gpio_t gpio, uint8_t state)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == GPIO_MODE_OUTPUT);

   switch(gpio_def[gpio].port)
   {
      case GPIO_PORTB:
         if (state)
            PORTB |= (1 << gpio_def[gpio].pin);
         else
            PORTB &= ~(1 << gpio_def[gpio].pin);
         break;
      case GPIO_PORTC:
         if (state)
            PORTC |= (1 << gpio_def[gpio].pin);
         else
            PORTC &= ~(1 << gpio_def[gpio].pin);
         break;
      case GPIO_PORTD:
         if (state)
            PORTD |= (1 << gpio_def[gpio].pin);
         else
            PORTD &= ~(1 << gpio_def[gpio].pin);
         break;
   }
}

void hal_gpio_toggle(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == GPIO_MODE_OUTPUT);
   
   switch(gpio_def[gpio].port)
   {
      case GPIO_PORTB:
         if (PORTB & (1 << gpio_def[gpio].pin))
            PORTB &= ~(1 << gpio_def[gpio].pin);
         else
            PORTB |= (1 << gpio_def[gpio].pin);
         break;
      case GPIO_PORTC:
         if (PORTC & (1 << gpio_def[gpio].pin))
            PORTC &= ~(1 << gpio_def[gpio].pin);
         else
            PORTC |= (1 << gpio_def[gpio].pin);
         break;
         break;
      case GPIO_PORTD:
         if (PORTD & (1 << gpio_def[gpio].pin))
            PORTD &= ~(1 << gpio_def[gpio].pin);
         else
            PORTD |= (1 << gpio_def[gpio].pin);
         break;
   }
}

uint8_t hal_gpio_get(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   // TODO:
   return 0;
}

int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler)
{
/*   
   // TODO: zobecnit na parametry
   ASSERT(gpio == BTN_MODE);
   
   btn_irq_handler = handler;

   HAL_NVIC_SetPriority(EXTI4_IRQn, BTN_MODE_PRIORITY, 0);
   HAL_NVIC_EnableIRQ(EXTI4_IRQn);   
*/   
   return 0;
}

/*
void EXTI4_IRQHandler(void)
{
   // TODO: zobecnit !!
   if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET) 
   { 
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
      
      if (btn_irq_handler != NULL)
         btn_irq_handler(BTN_MODE);
   }
}
*/