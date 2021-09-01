/*
  +-----+-----+----------+------+---+-NanoPi NEO/NEO2--+------+----------+-----+-----+
 | BCM | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi | BCM |
 +-----+-----+----------+------+---+----++----+---+------+----------+-----+-----+
 |     |     |     3.3V |      |   |  1 || 2  |   |      | 5V       |     |     |
 |  12 |   8 |  GPIOA12 |  OFF | 0 |  3 || 4  |   |      | 5V       |     |     |
 |  11 |   9 |  GPIOA11 |  OFF | 0 |  5 || 6  |   |      | 0v       |     |     |
 | 203 |   7 |  GPIOG11 |  OUT | 1 |  7 || 8  | 0 | OFF  | GPIOG6   | 15  | 198 |
 |     |     |       0v |      |   |  9 || 10 | 0 | OFF  | GPIOG7   | 16  | 199 |
 |   0 |   0 |   GPIOA0 |  OFF | 0 | 11 || 12 | 0 | OFF  | GPIOA6   | 1   | 6   |
 |   2 |   2 |   GPIOA2 |  OFF | 0 | 13 || 14 |   |      | 0v       |     |     |
 |   3 |   3 |   GPIOA3 |  OFF | 0 | 15 || 16 | 0 | OFF  | GPIOG8   | 4   | 200 |
 |     |     |     3.3v |      |   | 17 || 18 | 0 | OFF  | GPIOG9   | 5   | 201 |
 |  64 |  12 |   GPIOC0 |  OFF | 0 | 19 || 20 |   |      | 0v       |     |     |
 |  65 |  13 |   GPIOC1 |  OFF | 0 | 21 || 22 | 0 | OFF  | GPIOA1   | 6   | 1   |
 |  66 |  14 |   GPIOC2 |  OFF | 0 | 23 || 24 | 0 | OFF  | GPIOC3   | 10  | 67  |
 +-----+-----+----------+------+---+----++----+---+------+----------+-----+-----+
 | BCM | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi | BCM |
 +-----+-----+----------+------+---+-NanoPi NEO/NEO2--+------+----------+-----+-----+
 
 +-----+----NanoPi NEO/NEO2 Debug UART-+----+
 | BCM | wPi |   Name   | Mode | V | Ph |
 +-----+-----+----------+------+---+----+
 |   4 |  17 |   GPIOA4 | ALT5 | 0 | 37 |
 |   5 |  18 |   GPIOA5 | ALT4 | 0 | 38 |
 +-----+-----+----------+------+---+----+

*/

#include "system.h"

#include "wiringPi.h"

TRACE_TAG(hal_gpio);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

static const hal_gpio_def_t gpio_def[] = CFG_HAL_GPIO_DEF;
#define NUM_GPIO   (sizeof(gpio_def) / sizeof(hal_gpio_def_t))


int hal_gpio_init(void)
{
   int ix;

   wiringPiSetup() ;

   TRACE("GPIO configuration:");
   for (ix = 0; ix < NUM_GPIO; ix++)
   {
      TRACE("   GPIO[%d]  pin: %d  mode: %s", ix, gpio_def[ix].pin, gpio_def[ix].mode == OUTPUT ? "OUTPUT" : "INPUT");

      if (gpio_def[ix].mode == OUTPUT) {
         pinMode (gpio_def[ix].pin, OUTPUT) ;
         digitalWrite(gpio_def[ix].pin, gpio_def[ix].param);
      } else {
         pinMode (gpio_def[ix].pin, INPUT) ;
      }      
   }

   TRACE("Init");

   return 0;
}

/** Configure GPIO as input or output */
int hal_gpio_configure(hal_gpio_t gpio, hal_gpio_mode_t mode)
{
   if (mode == HAL_GPIO_MODE_OUTPUT) {
      pinMode (gpio_def[gpio].pin, OUTPUT) ;
      digitalWrite(gpio_def[gpio].pin, 0);
   } else {
      pinMode (gpio_def[gpio].pin, INPUT) ;
   }      
   
   return 0;
}

void hal_gpio_set(hal_gpio_t gpio, uint8_t state)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == OUTPUT);
   digitalWrite(gpio_def[gpio].pin, state);
}

void hal_gpio_toggle(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == OUTPUT);
   digitalWrite(gpio_def[gpio].pin, digitalRead(gpio_def[gpio].pin) ^ 0x1);
}

uint8_t hal_gpio_get(hal_gpio_t gpio)
{
   ASSERT(gpio < NUM_GPIO);
   ASSERT(gpio_def[gpio].mode == INPUT);
   return digitalRead(gpio_def[gpio].pin);
}

int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler)
{
   return 0;
}
