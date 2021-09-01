
#include "system.h"
#include "driver/rtc_io.h"

TRACE_TAG(hal_gpio);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

static const hal_gpio_def_t gpio_def[] = CFG_HAL_GPIO_DEF;
#define NUM_GPIO  (sizeof(gpio_def) / sizeof(hal_gpio_def_t))

static uint32_t gpio_output_state;

int hal_gpio_init(void)
{
   int gpio;

   gpio_output_state = 0;

   for (gpio = 0; gpio < NUM_GPIO; gpio++)
   {
      // Configure GPIO
      hal_gpio_configure(gpio, gpio_def[gpio].mode);

      // Set default output state
      if (gpio_def[gpio].mode == HAL_GPIO_MODE_OUTPUT)
         hal_gpio_set(gpio, gpio_def[gpio].default_output_state);
   }

   return 0;
}

/** Configure GPIO as input or output */
int hal_gpio_configure(hal_gpio_t gpio, hal_gpio_mode_t mode)
{
   gpio_config_t conf = {0};

   ASSERT(gpio < NUM_GPIO);

   if (gpio_def[gpio].pin == 33)
   {
        gpio_pad_select_gpio(33);
        rtc_gpio_deinit(33);
   }

   conf.pin_bit_mask = (1 << gpio_def[gpio].pin);

   switch(mode)
   {
      case HAL_GPIO_MODE_INPUT:
         conf.mode = GPIO_MODE_INPUT;
         break;

      case HAL_GPIO_MODE_INPUT_PULLUP:
         conf.mode = GPIO_MODE_INPUT;
         conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
         conf.pull_up_en = GPIO_PULLUP_ENABLE;
         break;

      case HAL_GPIO_MODE_OUTPUT:
         conf.mode = GPIO_MODE_OUTPUT;
         break;

      default:
         TRACE_ERROR("Not supported mode");
         return -1;
   }

   gpio_config(&conf);

   return 0;
}

void hal_gpio_set(hal_gpio_t gpio, uint8_t state)
{
   ASSERT(gpio < NUM_GPIO);

   if (state)
   {
      gpio_output_state |= (1 << gpio);
   }
   else
   {
      gpio_output_state &= ~(1 << gpio);
   }

   gpio_set_level(gpio_def[gpio].pin, state);
}

void hal_gpio_toggle(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   hal_gpio_set(gpio, (gpio_output_state & (1 << gpio)) ? 0 : 1);
}

uint8_t hal_gpio_get(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   return gpio_get_level(gpio_def[gpio].pin);
}

int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler)
{
   TRACE_ERROR("Not implemented");
   ASSERT(0);
   return 0;
}
