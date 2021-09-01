
#include "system.h"

#if defined (CPU_USB_DEVICE_MSC)

#include "class/msc/usb_device.h"


/** Initialize USB mass storage */
int hal_usbmsc_init(hal_usb_t dev)
{
   int res;
   RCC_PeriphCLKInitTypeDef PeriphClkInit;

   // Enable clock
   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
   PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
   HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

   // Init USB driver
   res = usb_device_init();

   return res;
}

/** Deinitialize USB serial port */
void hal_usbmsc_deinit(hal_usb_t dev)
{
}


#endif  // CPU_USB_DEVICE_MSC
