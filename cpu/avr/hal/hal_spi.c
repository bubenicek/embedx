
#include "system.h"

#define TRACE_TAG	"hal-spi"

#if !ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif


int hal_spi_init(hal_spi_t spi)
{
   return 0;
}

void hal_spi_set_speed(hal_spi_t spi, uint32_t speed)
{
}

uint8_t hal_spi_transmit(hal_spi_t spi, uint8_t c)
{
   return c;
}

void hal_spi_read(hal_spi_t spi, uint8_t *buf, int bufsize)
{  
}

void hal_spi_write(hal_spi_t spi, const uint8_t *buf, int bufsize)
{  
}
