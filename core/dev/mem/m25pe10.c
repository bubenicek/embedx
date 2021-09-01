
#include "system.h"
#include "m25pe10.h"

#define TRACE_TAG    "25PE10"
#if !ENABLE_TRACE_SPI_MEM
#undef TRACE
#define TRACE(...)
#endif

#ifndef M25PE10_SPI
#define M25PE10_SPI  DF_SPI
#endif // M25PE10_SPI

#ifndef M25PE10_SPI_CS
#define M25PE10_SPI_CS  DF_SPI_CS
#endif // M25PE10_SPI_CS



#define M25PE10_PAGE_SIZE            256
#define M25PE10_SIZE                 (128 * 1024)
#define M25PE10_NUM_SECTORS          2

#define M25PE10_CMD_RDSR            0x05
#define M25PE10_CMD_WREN            0x06
#define M25PE10_CMD_SECTOR_ERASE    0xD8


static uint8_t m25pe10_get_status(void);


int m25pe10_init(void)
{
   if (hal_spi_init(M25PE10_SPI) != 0)
   {
      TRACE_ERROR("Init M95256_SPI");
      return -1;
   }

   return 0;
}

int m25pe10_read(uint16_t addr, uint8_t *buf, int count)
{
   return 0;
}

int m25pe10_write(uint16_t addr, uint8_t *buf, int count)
{
   return 0;
}

int m25pe10_erase_sector(uint32_t addr)
{
   // Enable WR
   hal_spi_select(M25PE10_SPI, M25PE10_SPI_CS);
   hal_spi_transmit(M25PE10_SPI, M25PE10_CMD_WREN);
   hal_spi_deselect(M25PE10_SPI, M25PE10_SPI_CS);

   // Erase sector
   hal_spi_select(M25PE10_SPI, M25PE10_SPI_CS);
   hal_spi_transmit(M25PE10_SPI, M25PE10_CMD_SECTOR_ERASE);
   hal_spi_transmit(M25PE10_SPI, addr >> 16);
   hal_spi_transmit(M25PE10_SPI, addr >> 8);
   hal_spi_transmit(M25PE10_SPI, addr & 0xFF);
   hal_spi_deselect(M25PE10_SPI, M25PE10_SPI_CS);

   while(m25pe10_get_status() & 0x1);

   return 0;
}


int m25pe10_erase_chip(void)
{
   TRACE("Erase start");
   m25pe10_erase_sector(0x0);
   m25pe10_erase_sector(0x1000);
   TRACE("Erase done");

   return 0;
}

static uint8_t m25pe10_get_status(void)
{
   uint8_t res;

   hal_spi_select(M25PE10_SPI, M25PE10_SPI_CS);
   hal_spi_transmit(M25PE10_SPI, M25PE10_CMD_RDSR);
   res = hal_spi_transmit(M25PE10_SPI, 0xFF);
   hal_spi_deselect(M25PE10_SPI, M25PE10_SPI_CS);

   return res;
}