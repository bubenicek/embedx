
#include "system.h"

#if defined (CPU_USB_DEVICE_CDC)
#include "usbd_cdc_if.h"

#define TRACE_TAG "usbserial"
#if !ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

/** Initialize USB serial port */
int hal_usbserial_init(hal_usb_t dev)
{
   int res;
   RCC_PeriphCLKInitTypeDef PeriphClkInit;

   // Enable clock
   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
   PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
   HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

   // Init USB driver
   usb_device_init();
   
   // Wait some time
   hal_delay_ms(1000);
   
   TRACE("USB serial init");

   return 0;
}

/** Deinitialize USB serial port */
void hal_usbserial_deinit(hal_usb_t dev)
{

}

/** Put char */
void hal_usbserial_putchar(hal_uart_t uart, uint8_t c)
{
   CDC_Transmit(&c, 1);
}

/** Get char */
int hal_usbserial_getchar(hal_uart_t uart)
{
   uint8_t c;
   
   return (CDC_Receive(&c, 1) > 0) ? c : -1;
}

/** Write buffer to USB serial port */
int hal_usbserial_write(hal_usb_t dev, void *buf, uint16_t count)
{
   return 0;
}

/** Read buffer from USB serial port*/
int hal_usbserial_read(hal_usb_t dev, void *buf, uint16_t count, uint16_t timeout)
{
   return 0;
}

#endif  // CPU_USB_DEVICE_CDC
