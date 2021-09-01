
#ifndef __HAL_DEF_H
#define __HAL_DEF_H


//
// GPIO
//
typedef struct
{
   uint32_t pin;
   uint8_t mode;
   uint8_t param;  // Output type = default output state, Input type = SUNXI_PULL_NONE|SUNXI_PULL_UP|SUNXI_PULL_DOWN

} hal_gpio_def_t;


//
// UART
//
typedef struct
{
   const char *devname;    // Device name
   uint32_t baudrate;
   uint32_t settings;

   int fd;                 // Socket descriptor

} hal_uart_def_t;


#endif   // __HAL_DEF_H
