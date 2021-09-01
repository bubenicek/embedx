
#include "system.h"

TRACE_TAG(hal_led);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif


#if defined (CFG_OPENOS_OS_API) && (CFG_OPENOS_OS_API == 1)

//
// OPENOS
//

/** LED state */
typedef struct
{
   hal_gpio_t gpio;
   uint8_t high_state;
   os_timer_t period_timer;
   os_timer_t duty_timer;
   int count;
   int time;
   int duty;

} led_state_t;

// Prototypes:
static void led_timer_duty_callback(void *arg);
static void led_timer_period_callback(void *arg);


// Locals:
static led_state_t ledstate[] = CFG_HAL_LED_DEF;
#define NUM_LEDS   (sizeof(ledstate) / sizeof(led_state_t))


void hal_led_set(hal_led_t led, uint8_t state)
{
   ASSERT(led < NUM_LEDS);
   hal_led_blink(led, 0, 0, 0);  // LED off
   hal_gpio_set(ledstate[led].gpio, state ? ledstate[led].high_state : ledstate[led].high_state ^ 1);
}

void hal_led_toggle(hal_led_t led)
{
   ASSERT(led < NUM_LEDS);
   hal_gpio_toggle(ledstate[led].gpio);
}

void hal_led_blink(hal_led_t led, int count, int duty, int time)
{
   led_state_t *pstate;

   ASSERT(led < NUM_LEDS);
   pstate = &ledstate[led];

   pstate->count = count;
   pstate->time = time;
   pstate->duty = duty;

   if (time > 0)
   {
      if (pstate->count != -1)
         pstate->count--;

      if (pstate->count != 0)
         os_timer_start(&pstate->period_timer, OS_TIMER_IRQ_PERIODICAL, pstate->time, led_timer_period_callback, pstate);

      hal_gpio_set(pstate->gpio, pstate->high_state);  // LED on

      os_timer_start(&pstate->duty_timer, OS_TIMER_IRQ_ONESHOT, pstate->duty, led_timer_duty_callback, pstate);
   }
   else
   {
      // Stop timers
      os_timer_stop(&pstate->period_timer);
      os_timer_stop(&pstate->duty_timer);
   }
}

/** LED duty timer callback */
static void led_timer_duty_callback(void *arg)
{
   led_state_t *pstate = arg;
   hal_gpio_set(pstate->gpio, pstate->high_state ^ 1);  // LED off
}

/** LED period timer callback */
static void led_timer_period_callback(void *arg)
{
   led_state_t *pstate = arg;

   if (pstate->count != -1)
      pstate->count--;

   if (pstate->count == 0)
      os_timer_stop(&pstate->period_timer);

   hal_gpio_set(pstate->gpio, pstate->high_state);  // LED on

   os_timer_start(&pstate->duty_timer, OS_TIMER_IRQ_ONESHOT, pstate->duty, led_timer_duty_callback, pstate);
}

#elif defined (CFG_CMSIS_OS_API) && (CFG_CMSIS_OS_API == 1)

//
// CMSIS API
//

#include "cmsis_os.h"

// Prototypes:
static inline void __led_set(hal_led_t led, uint8_t state);
static void led_timer_duty_callback(void *arg);
static void led_timer_period_callback(void *arg);


/** LED definition and state */
typedef struct
{
   uint32_t gpio;
   uint8_t active_state;
   uint8_t state;

   hal_led_t led;
   osTimerId period_timer;
   osTimerId duty_timer;
   int count;
   int period_time;
   int duty_time;

} led_def_t;

static led_def_t leds[] = CFG_HAL_LED_DEF;
#define NUM_LEDS     (sizeof(leds) / sizeof(led_def_t))

static const osTimerDef(PERIOD_TIMER, led_timer_period_callback);
static const osTimerDef(DUTY_TIMER, led_timer_duty_callback);



/**
 * Set LED blink
 * @param ioctl ioctl task context
 * @param gpio LED gpio
 * @param count number of period count (-1 = infinite)
 * @param duty duty time of led is high
 * @param time period time in ms
 */
void hal_led_blink(hal_led_t led, int count, int duty_time, int period_time)
{
   led_def_t *pstate;

   ASSERT(led < NUM_LEDS);
   ASSERT(duty_time < period_time);

   pstate = &leds[led];

   if (period_time > 0)
   {
      pstate->led = led;
      pstate->count = count;
      pstate->period_time = period_time;
      pstate->duty_time = duty_time;

      if (pstate->period_timer == NULL)
      {
         // Create period timer
         pstate->period_timer = osTimerCreate(osTimer(PERIOD_TIMER), osTimerPeriodic, pstate);
         if (pstate->period_timer == NULL)
         {
            TRACE_ERROR("Create LED[%d] period timer failed", led);
            return;
         }

         // Create duty timer
         pstate->duty_timer = osTimerCreate(osTimer(DUTY_TIMER), osTimerOnce, pstate);
         if (pstate->duty_timer == NULL)
         {
            TRACE_ERROR("Create LED[%d] duty timer failed", led);
            return;
         }
      }

      osTimerStart(pstate->period_timer, period_time);     
   }
   else
   {
      // Stop period timer
      if (pstate->period_timer != NULL)
         osTimerStop(pstate->period_timer);
   }
}

/** Set LED ON/OFF */
void hal_led_set(hal_led_t led, uint8_t state)
{
   ASSERT(led < NUM_LEDS);

   if (!state)
   {
      if (leds[led].period_timer != NULL)
         osTimerStop(leds[led].period_timer);

      if (leds[led].duty_timer != NULL)
         osTimerStop(leds[led].duty_timer);
   }

   __led_set(led, state);
}

/** Toggle LED output */
void hal_led_toggle(hal_led_t led)
{
   ASSERT(led < NUM_LEDS);
   hal_led_set(led, leds[led].state ^ 0x1);
}


static inline void __led_set(hal_led_t led, uint8_t state)
{
   ASSERT(led < NUM_LEDS);
   leds[led].state = state;
   hal_gpio_set(leds[led].gpio, state ? leds[led].active_state : leds[led].active_state ^ 1);
}



/** LED duty timer callback */
static void led_timer_duty_callback(void *arg)
{
   led_def_t *pstate = arg;
   ASSERT(pstate != NULL);
   __led_set(pstate->led, 0);
}

/** LED period timer callback */
static void led_timer_period_callback(void *arg)
{
   led_def_t *pstate = arg;

   ASSERT(pstate != NULL);

   if (pstate->count != -1)
      pstate->count--;

   if (pstate->count == 0)
   {
      osTimerStop(pstate->period_timer);
   }

   __led_set(pstate->led, 1);
   osTimerStart(pstate->duty_timer, pstate->duty_time);
}

#else

//
// LED blink disabled
//

/** LED state */
typedef struct
{
   hal_gpio_t gpio;
   uint8_t active_state;

} led_state_t;

// Locals:
static const led_state_t ledstate[] = CFG_HAL_LED_DEF;
#define NUM_LEDS   (sizeof(ledstate) / sizeof(led_state_t))

void hal_led_set(hal_led_t led, uint8_t state)
{
   ASSERT(led < NUM_LEDS);
   hal_gpio_set(ledstate[led].gpio, state ? ledstate[led].active_state : ledstate[led].active_state ^ 1);
}

void hal_led_toggle(hal_led_t led)
{
   ASSERT(led < NUM_LEDS);
   hal_gpio_toggle(ledstate[led].gpio);
}

void hal_led_blink(hal_led_t led, int count, int duty, int time)
{
}

#endif   // CFG_OPENOS_OS_API

