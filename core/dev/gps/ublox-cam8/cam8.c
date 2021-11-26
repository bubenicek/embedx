
#include "system.h"

TRACE_TAG(gps_cam8);

#ifndef CFG_GPS_CAM8_RXBUFSIZE
#define CFG_GPS_CAM8_RXBUFSIZE      1024
#endif

static uint8_t rxbuf[CFG_GPS_CAM8_RXBUFSIZE];
static int rxbuf_rindex;

/** Initialize GPS driver */
int gps_driver_init(void)
{
	// Enable power
 	hal_gpio_set(GPS_PWR_EN, 0);

	if (hal_spi_init(GPS_SPI) != 0)
   {
      TRACE_ERROR("Init GPS spi[%d] failed", GPS_SPI);
      return -1;
   }

   // Buffer is empty
   rxbuf_rindex = CFG_GPS_CAM8_RXBUFSIZE;

   TRACE("Init");

   return 0;
}

int gps_driver_deinit(void)
{
   // Disable power
 	hal_gpio_set(GPS_PWR_EN, 1);
   return 0;
}

/** Read NMEA data from GPS driver */
int gps_driver_read(char *buf, int bufsize)
{
   int total = 0;

   if (rxbuf_rindex == CFG_GPS_CAM8_RXBUFSIZE)
   {
      memset(rxbuf, 0xFF, sizeof(rxbuf));
      hal_spi_read(GPS_SPI, rxbuf, sizeof(rxbuf));
      rxbuf_rindex = 0;
   }

   while (rxbuf_rindex < CFG_GPS_CAM8_RXBUFSIZE && bufsize > 0)
   {
      if (rxbuf[rxbuf_rindex] != 0xFF)
      {
         *buf++ = rxbuf[rxbuf_rindex];
         bufsize--;
         total++;
      }
      rxbuf_rindex++;
   }

   return total;
}

/** Write data to GPS driver */
int gps_driver_write(char *buf, int bufsize)
{
    return hal_spi_write(GPS_SPI, buf, bufsize);
}
