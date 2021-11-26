
#include "system.h"

TRACE_TAG(gps_uart);

// Locals:

/** Initialize GPS driver */
int gps_driver_init(void)
{
   if (hal_uart_init(GPS_UART) != 0)
   {
      TRACE_ERROR("Uart[%d] init failed", GPS_UART);
      return -1;
   }

   return 0;
}

/** Read NMEA data from GPS driver */
int gps_driver_read(char *buf, int bufsize)
{
   return hal_uart_read(GPS_UART, (unsigned char *)buf, 1, 0);
}

/** Write data to GPS driver */
int gps_driver_write(char *buf, int bufsize)
{
   return hal_uart_write(GPS_UART, (unsigned char *)buf, bufsize);
}

