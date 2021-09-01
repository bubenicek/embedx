
#include "system.h"

#ifdef CFG_HAL_SPI_DEF

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

TRACE_TAG(hal_spi);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

typedef struct
{
   int fd;

} hal_spi_device_t;


// Locals:
static hal_spi_def_t spidef[] = CFG_HAL_SPI_DEF;
#define NUM_SPI   (sizeof(spidef) / sizeof(hal_spi_def_t))
static hal_spi_device_t spidev[NUM_SPI];


/** Initialize SPI */
int hal_spi_init(hal_spi_t spi)
{
   ASSERT(spi < NUM_SPI);

   spidev[spi].fd = -1;
   if ((spidev[spi].fd = open(spidef[spi].devname, O_RDWR)) < 0)
   {
      TRACE_ERROR("Open SPI device '%s' failed", spidef[spi].devname);
      throw_exception(fail);
   }

	if (ioctl(spidev[spi].fd, SPI_IOC_WR_MODE, &spidef[spi].mode) < 0)
   {
      TRACE_ERROR("Set SPI device '%s' mode failed", spidef[spi].devname);
      throw_exception(fail);
   }
	if (ioctl(spidev[spi].fd, SPI_IOC_RD_MODE, &spidef[spi].mode) < 0)
   {
      TRACE_ERROR("Set SPI device '%s' mode failed", spidef[spi].devname);
      throw_exception(fail);
   }

	if (ioctl(spidev[spi].fd, SPI_IOC_WR_BITS_PER_WORD, &spidef[spi].bits) < 0)
   {
      TRACE_ERROR("Set SPI '%s' bits failed", spidef[spi].devname);
      throw_exception(fail);
   }
	if (ioctl(spidev[spi].fd, SPI_IOC_RD_BITS_PER_WORD, &spidef[spi].bits) < 0)
   {
      TRACE_ERROR("Set SPI '%s' bits failed", spidef[spi].devname);
      throw_exception(fail);
   }

	if (ioctl(spidev[spi].fd, SPI_IOC_WR_MAX_SPEED_HZ, &spidef[spi].speed) < 0)
   {
      TRACE_ERROR("Set SPI '%s' speed failed", spidef[spi].devname);
      throw_exception(fail);
   }
	if (ioctl(spidev[spi].fd, SPI_IOC_RD_MAX_SPEED_HZ, &spidef[spi].speed) < 0)
   {
      TRACE_ERROR("Set SPI '%s' speed failed", spidef[spi].devname);
      throw_exception(fail);
   }

   TRACE("SPI %s speed: %d  mode: %d  bits: %d  init", spidef[spi].devname, spidef[spi].speed, spidef[spi].mode, spidef[spi].bits);

   return 0;

fail:
   if (spidev->fd != -1)
      close(spidev->fd);

   return -1;
}

int hal_spi_set_speed(hal_spi_t spi, uint32_t speed)
{
   ASSERT(spi < NUM_SPI);
   return 0;
}

static int __hal_spi_transmit(hal_spi_t spi, const uint8_t *txbuf, uint8_t *rxbuf, int bufsize) 
{
   ASSERT(spi < NUM_SPI);
   struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)txbuf,
        .rx_buf = (unsigned long)rxbuf,
		.len = bufsize,
		.delay_usecs = 0,
	};

	return ioctl(spidev[spi].fd, SPI_IOC_MESSAGE(1), &tr);
}

int hal_spi_transmit(hal_spi_t spi, uint8_t c)
{
   uint8_t b;

   if (__hal_spi_transmit(spi, &c, &b, 1) < 0)
   {
      TRACE_ERROR("Transmit byte failed, errno: %d", errno);
      return -1;
   }

   return b;
}

int hal_spi_read(hal_spi_t spi, uint8_t *buf, int bufsize)
{
   if (__hal_spi_transmit(spi, buf, buf, bufsize) < 0)
   {
      TRACE_ERROR("Read failed, errno: %d", errno);
      return -1;
   }
 
   return bufsize;
}

int hal_spi_write(hal_spi_t spi, const uint8_t *buf, int bufsize)
{
   if (__hal_spi_transmit(spi, buf, (uint8_t *)buf, bufsize) < 0)
   {
      TRACE_ERROR("Write failed, errno: %d", errno);
      return -1;
   }
 
   return bufsize;
}

/** Send burst buffer to SPI with cs state */
int hal_spi_write_burst(hal_spi_t spi, const uint8_t *buf, int bufsize, bool last)
{
   ASSERT(spi < NUM_SPI);
   struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)buf,
		.len = bufsize,
		.delay_usecs = 0,
        .cs_change = last ? false : true,    // Set CS=HIGH on last write
	};

	if (ioctl(spidev[spi].fd, SPI_IOC_MESSAGE(1), &tr) < 0)
   {
      TRACE_ERROR("Write failed, errno: %d", errno);
      return -1;
   }

   return bufsize;
}

#endif  // CFG_HAL_SPI_DEF