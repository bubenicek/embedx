
#include "system.h"
#include "sunxi_gpio.h"

TRACE_TAG(hal_gpio);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

static const hal_gpio_def_t gpio_def[] = CFG_HAL_GPIO_DEF;
#define NUM_GPIO   (sizeof(gpio_def) / sizeof(hal_gpio_def_t))


int hal_gpio_init(void)
{
   int ix, res = 0;

   if (sunxi_gpio_init() != 0)
   {
      TRACE_ERROR("sunxi gpio init failed ");
      return -1;
   }

   for (ix = 0; ix < NUM_GPIO; ix++)
   {
      if (gpio_def[ix].mode == SUNXI_GPIO_OUTPUT)
      {
         res += sunxi_gpio_set_cfgpin(gpio_def[ix].pin, SUNXI_GPIO_OUTPUT);
         res += sunxi_gpio_output(gpio_def[ix].pin, gpio_def[ix].param);
      }
      else
      {
         res += sunxi_gpio_set_cfgpin(gpio_def[ix].pin, SUNXI_GPIO_INPUT);
         res += sunxi_gpio_pullup(gpio_def[ix].pin, gpio_def[ix].param);
      }
   }

   return res;
}

/** Configure GPIO as input or output */
int hal_gpio_configure(hal_gpio_t gpio, hal_gpio_mode_t mode)
{
   ASSERT(gpio < NUM_GPIO);

   if (mode == HAL_GPIO_MODE_OUTPUT)
   {
      sunxi_gpio_set_cfgpin(gpio_def[gpio].pin, SUNXI_GPIO_OUTPUT);
   }
   else
   {
      uint8_t pull;

      switch(mode)
      {
         case HAL_GPIO_MODE_INPUT_PULLUP:
            pull = SUNXI_PULL_UP;
            break;

         case HAL_GPIO_MODE_INPUT_PULLDOWN:
            pull = SUNXI_PULL_DOWN;
            break;

         default:
            pull = SUNXI_PULL_NONE;
      }

      sunxi_gpio_set_cfgpin(gpio_def[gpio].pin, SUNXI_GPIO_INPUT);
      sunxi_gpio_pullup(gpio_def[gpio].pin, pull);
   }

   return 0;
}


void hal_gpio_set(hal_gpio_t gpio, uint8_t state)
{
   ASSERT(gpio < NUM_GPIO);
   sunxi_gpio_output(gpio_def[gpio].pin, state);
}

void hal_gpio_toggle(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   sunxi_gpio_output(gpio_def[gpio].pin, sunxi_gpio_input(gpio_def[gpio].pin) ^ 1);
}

uint8_t hal_gpio_get(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   return sunxi_gpio_input(gpio_def[gpio].pin);
}

int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler)
{
   return 0;
}
