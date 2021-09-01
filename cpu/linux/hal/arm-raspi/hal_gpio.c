
#include "system.h"

TRACE_TAG(hal_gpio);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

static const hal_gpio_def_t gpio_def[] = CFG_HAL_GPIO_DEF;
#define NUM_GPIO   (sizeof(gpio_def) / sizeof(hal_gpio_def_t))


int hal_gpio_init(void)
{

   TRACE("Init");

   return 0;
}

int hal_gpio_configure(hal_gpio_t gpio, hal_gpio_mode_t mode)
{
   return 0;
}

void hal_gpio_set(hal_gpio_t gpio, uint8_t state)
{
}

void hal_gpio_toggle(hal_gpio_t gpio)
{
}

uint8_t hal_gpio_get(hal_gpio_t gpio)
{
   return 0;
}

int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler)
{
   return 0;
}
