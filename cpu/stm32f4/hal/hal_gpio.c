
#include "system.h"

TRACE_TAG(hal_gpio);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

static const hal_gpio_def_t gpio_def[] = CFG_HAL_GPIO_DEF;
#define NUM_GPIO   (sizeof(gpio_def) / sizeof(hal_gpio_def_t))

/** IRQ handler */
static hal_gpio_irq_handler_t gpio_irq_handlers[NUM_GPIO];

/** Prev states */
static uint8_t gpio_prev_states[NUM_GPIO];


int hal_gpio_init(void)
{
   int ix;
   GPIO_InitTypeDef GPIO_InitStructure;

   memset(&gpio_irq_handlers, 0, sizeof(gpio_irq_handlers));

   GPIO_StructInit(&GPIO_InitStructure);
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

   for (ix = 0; ix < NUM_GPIO; ix++)
   {
      GPIO_InitStructure.GPIO_Pin = gpio_def[ix].pin;
      GPIO_InitStructure.GPIO_Mode = gpio_def[ix].mode;
      if (gpio_def[ix].mode == GPIO_Mode_OUT)
      {
         GPIO_WriteBit(gpio_def[ix].port, gpio_def[ix].pin, gpio_def[ix].init_output_state);
      }
      GPIO_Init(gpio_def[ix].port, &GPIO_InitStructure);
   }

   return 0;
}

void hal_gpio_set(hal_gpio_t gpio, uint8_t state)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == GPIO_Mode_OUT);

   GPIO_WriteBit(gpio_def[gpio].port, gpio_def[gpio].pin, state);
}

void hal_gpio_toggle(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == GPIO_Mode_OUT);

   GPIO_ToggleBits(gpio_def[gpio].port, gpio_def[gpio].pin);
}

uint8_t hal_gpio_get(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == GPIO_Mode_IN);

   return GPIO_ReadInputDataBit(gpio_def[gpio].port, gpio_def[gpio].pin);
}

int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler)
{
   EXTI_InitTypeDef EXTI_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == GPIO_Mode_IN);

   DISABLE_INTERRUPTS();

   gpio_irq_handlers[gpio] = handler;

   // Connect EXTI Line to INT Pin
   SYSCFG_EXTILineConfig(gpio_def[gpio].exti_port_source, gpio_def[gpio].exti_pin_source);

   // Configure EXTI line
   EXTI_InitStructure.EXTI_Line = gpio_def[gpio].exti_line;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   switch(edge)
   {
      case HAL_GPIO_IRQ_EDGE_RISING:
         EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
         break;
      case HAL_GPIO_IRQ_EDGE_FALLING:
         EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
         break;
      default:
         EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
   }

   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   // Enable and set the EXTI interrupt to the highest priority
   NVIC_InitStructure.NVIC_IRQChannel = gpio_def[gpio].exti_irqn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CFG_HAL_GPIO_IRQ_PRIORITY;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   ENABLE_INTERRUPTS();

   return 0;
}

static inline void gpio_irq_handler(void)
{
   int ix;

   for (ix = 0; ix < NUM_GPIO; ix++)
   {
      if (gpio_def[ix].mode == GPIO_Mode_IN && EXTI_GetITStatus(gpio_def[ix].exti_line) != RESET)
      {
         if (gpio_irq_handlers[ix] != NULL)
         {
            if (hal_gpio_get(ix) != gpio_prev_states[ix]) 
            {
               gpio_irq_handlers[ix](ix);
               gpio_prev_states[ix] = hal_gpio_get(ix);
            }
         }
      }
   }

   // Clear interrupt pending bit
   for (ix = 0; ix < NUM_GPIO; ix++) 
   {
      if (EXTI_GetITStatus(gpio_def[ix].exti_line) != RESET)
         EXTI_ClearITPendingBit(gpio_def[ix].exti_line);
   }
}


void EXTI0_IRQHandler(void) {
   gpio_irq_handler();
}

void EXTI1_IRQHandler(void) {
   gpio_irq_handler();
}

void EXTI2_IRQHandler(void) {
   gpio_irq_handler();
}

void EXTI3_IRQHandler(void) {
   gpio_irq_handler();
}

void EXTI4_IRQHandler(void) {
   gpio_irq_handler();
}

void EXTI9_5_IRQHandler(void) {
   gpio_irq_handler();
}

void EXTI15_10_IRQHandler(void)
{
   gpio_irq_handler();
}
