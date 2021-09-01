
#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

#define ON  1
#define OFF 0

/** GPIO types */
typedef enum
{
	HAL_GPIO0 = 0,
	HAL_GPIO1,
	HAL_GPIO2,
	HAL_GPIO3,
	HAL_GPIO4,
	HAL_GPIO5,
	HAL_GPIO6,
	HAL_GPIO7,
	HAL_GPIO8,
	HAL_GPIO9,
	HAL_GPIO10,
	HAL_GPIO11,
	HAL_GPIO12,
	HAL_GPIO13,
	HAL_GPIO14,
	HAL_GPIO15,
	HAL_GPIO16,
	HAL_GPIO17,
	HAL_GPIO18,
	HAL_GPIO19,
	HAL_GPIO20,
	HAL_GPIO21,
	HAL_GPIO22,
	HAL_GPIO23,
	HAL_GPIO24,
	HAL_GPIO25,
	HAL_GPIO26,
	HAL_GPIO27,
	HAL_GPIO28,
	HAL_GPIO29,
	HAL_GPIO30,
	HAL_GPIO31,
	HAL_GPIO32,
	HAL_GPIO33,
	HAL_GPIO34,
	HAL_GPIO35,
	HAL_GPIO36,
	HAL_GPIO37,
	HAL_GPIO38,
	HAL_GPIO39,
	HAL_GPIO40,
	HAL_GPIO41,
	HAL_GPIO42,
	HAL_GPIO43,
	HAL_GPIO44,
	HAL_GPIO45,
	HAL_GPIO46,
	HAL_GPIO47,
	HAL_GPIO48,
	HAL_GPIO49,
	HAL_GPIO50,
	HAL_GPIO51,
	HAL_GPIO52,
	HAL_GPIO53,
	HAL_GPIO54,
	HAL_GPIO55,
	HAL_GPIO56,
	HAL_GPIO57,
	HAL_GPIO58,
	HAL_GPIO59,
	HAL_GPIO60,
	HAL_GPIO61,
	HAL_GPIO62,
	HAL_GPIO63,
	HAL_GPIO64,
	HAL_GPIO65,
	HAL_GPIO66,

} hal_gpio_t;


/** IRQ edge types */
typedef enum
{
   HAL_GPIO_IRQ_EDGE_RISING,
   HAL_GPIO_IRQ_EDGE_FALLING,
   HAL_GPIO_IRQ_EDGE_RISINGFALLING

} hal_gpio_irq_edge_t;

/** GPIO mode */
typedef enum
{
   HAL_GPIO_MODE_INPUT,
   HAL_GPIO_MODE_INPUT_PULLUP,
   HAL_GPIO_MODE_INPUT_PULLDOWN,
   HAL_GPIO_MODE_OUTPUT,

} hal_gpio_mode_t;


/** GPIO IRQ handler type */
typedef void (*hal_gpio_irq_handler_t)(hal_gpio_t gpio);

/** Initialise GPIO */
int hal_gpio_init(void);

/** Configure GPIO as input or output */
int hal_gpio_configure(hal_gpio_t gpio, hal_gpio_mode_t mode);

/** Set GPIO high/low */
void hal_gpio_set(hal_gpio_t gpio, uint8_t state);

/** Toggle GPIO */
void hal_gpio_toggle(hal_gpio_t gpio);

/** Get GPIO input state */
uint8_t hal_gpio_get(hal_gpio_t gpio);

/** Register GPIO interrupt handler */
int hal_gpio_register_irq_handler(hal_gpio_t gpio, hal_gpio_irq_edge_t edge, hal_gpio_irq_handler_t handler);

#endif // __HAL_GPIO_H
