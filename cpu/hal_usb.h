
#ifndef __HAL_USB_H
#define __HAL_USB_H

typedef enum
{
   HAL_USB0,
   HAL_USB1

} hal_usb_t;


//
// USB serial interface
//

/** Initialize USB serial port */
int hal_usbserial_init(hal_usb_t dev);

/** Deinitialize USB serial port */
void hal_usbserial_deinit(hal_usb_t dev);

/** Put char */
void hal_usbserial_putchar(hal_uart_t uart, uint8_t c);

/** Get char */
int hal_usbserial_getchar(hal_uart_t uart);

/** Write buffer to USB serial port */
int hal_usbserial_write(hal_usb_t dev, void *buf, uint16_t count);

/** Read buffer from USB serial port*/
int hal_usbserial_read(hal_usb_t dev, void *buf, uint16_t count, uint16_t timeout);


//
// USB mass storage interface
//

/** Initialize USB mass storage */
int hal_usbmsc_init(hal_usb_t dev);

/** Deinitialize USB mass storage */
void hal_usbmsc_deinit(hal_usb_t dev);


#endif   // __HAL_USB_H