
#include "system.h"

#if defined (CPU_USB_DEVICE_CDC)
#include "usb_device.h"
#include "usbd_cdc_if.h"

TRACE_TAG(usbserial)
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
   if (usb_device_init() != 0)
   {
       TRACE_ERROR("USB device init failed");
       return -1;
   }
    
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
    while(CDC_Transmit(&c, 1) == USBD_BUSY);
}

/** Get char */
int hal_usbserial_getchar(hal_uart_t uart)
{
   uint8_t c;
   
    if (CDC_Receive(&c, 1) > 0) 
    {
        //TRACE("RX: [%02X %c]", c, c);
        return c;
    }
    else
    {
        return -1;
    }
}

/** Write buffer to USB serial port */
int hal_usbserial_write(hal_usb_t dev, void *buf, uint16_t count)
{
    // TODO:
}

/** Read buffer from USB serial port*/
int hal_usbserial_read(hal_usb_t dev, void *buf, uint16_t count, uint16_t timeout)
{
    // TODO:
   return 0;
}

#endif  // CPU_USB_DEVICE_CDC
