
#ifndef __HAL_DEF_H
#define __HAL_DEF_H

//
// GPIO definition
//
typedef struct
{
   GPIO_TypeDef *port;
   uint16_t pin;
   GPIOMode_TypeDef mode;
   uint8_t init_output_state;

   uint32_t exti_line;
   uint32_t exti_port_source;
   uint32_t exti_pin_source;
   uint8_t  exti_irqn;

} hal_gpio_def_t;


//
// SPI definition
//

/** SPI pin definition */
typedef struct
{
   uint16_t pin;
   GPIO_TypeDef *port;
   uint8_t alternate_function;

} hal_spi_pin_def_t;


/** SPI device definition */
typedef struct
{
   SPI_TypeDef *spidev;
   uint32_t baudrate;
   hal_rcc_periph_bus_t bus_periph;
   uint32_t clk_periph;
#define NUM_SPI_PINS    3
   hal_spi_pin_def_t pins[NUM_SPI_PINS];   // MISO, MOSI, CLK

} hal_spi_def_t;


//
// UART definition
//
/** UART pin definition */
typedef struct
{
   uint16_t pin;
   GPIO_TypeDef *port;
   uint8_t alternate_function;

} hal_uart_pin_def_t;


/** UART definition */
typedef struct
{
   USART_TypeDef *usart;
   uint32_t baudrate;
   hal_rcc_periph_bus_t bus_periph;
   uint32_t clk_periph;
   uint16_t irqn;
#define NUM_UART_PINS   2
   hal_uart_pin_def_t pins[NUM_UART_PINS];   // TX, RX

} hal_uart_def_t;


//
// ADC
//
typedef struct
{
   ADC_TypeDef *dev;
   uint8_t channel;

} hal_adc_def_t;



#endif   // __HAL_DEF_H
