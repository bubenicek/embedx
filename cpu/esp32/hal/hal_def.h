
#ifndef __HAL_DEF_H
#define __HAL_DEF_H

//
// GPIO
//
typedef struct
{
   uint32_t pin;
   uint32_t mode;
   uint8_t default_output_state;

} hal_gpio_def_t;


//
// UART
//
typedef struct
{
   uint32_t uart;
   uint32_t baudrate;
   uint32_t rxpin;
   uint32_t txpin;

} hal_uart_def_t;


//
// I2C
//
typedef struct
{
   uint32_t sda_pin;
   uint32_t scl_pin;

} hal_i2c_def_t;


#endif   // __HAL_DEF_H
