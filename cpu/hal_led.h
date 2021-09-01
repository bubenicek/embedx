
#ifndef __HAL_LED_H
#define __HAL_LED_H

/** GPIO types */
typedef enum
{
	HAL_LED0,
	HAL_LED1,
	HAL_LED2,
	HAL_LED3,
	HAL_LED4,
	HAL_LED5,
	HAL_LED6,
	HAL_LED7,
	HAL_LED8,
	HAL_LED9,
	HAL_LED10,
	HAL_LED11,
	HAL_LED12,
	HAL_LED13,
	HAL_LED14,
	HAL_LED15,

} hal_led_t;

void hal_led_set(hal_led_t led, uint8_t state);

void hal_led_toggle(hal_led_t led);

void hal_led_blink(hal_led_t led, int count, int duty, int time);

#endif // __HAL_LED_H
