
#include "system.h"
#include "m95256.h"

#define TRACE_TAG    "M95256"

#ifndef M95256_SPI
#define M95256_SPI      DF_SPI
#endif // M95256_SPI

#ifndef M95256_SPI_CS
#define M95256_SPI_CS      DF_SPI_CS
#endif // M95256_SPI


#define M95256_CMD_WRSR             1
#define M95256_CMD_WRITE            2
#define M95256_CMD_READ             3
#define M95256_CMD_RDSR             5
#define M95256_CMD_WREN             6

#define M95256_PAGE_SIZE            64
#define M95256_SIZE                 32768


static uint8_t m95256_get_status(void);

int m95256_init(void)
{
   if (hal_spi_init(M95256_SPI) != 0)
   {
      TRACE_ERROR("Init M95256_SPI");
      return -1;
   }

   return 0;
}

int m95256_read(uint16_t addr, uint8_t *buf, int count)
{
   uint8_t cmd[] = {M95256_CMD_READ, addr >> 8, addr & 0xFF};

   hal_spi_select(M95256_SPI, M95256_SPI_CS);
   hal_spi_write(M95256_SPI, cmd, sizeof(cmd));
   hal_spi_read(M95256_SPI, buf, count);
   hal_spi_deselect(M95256_SPI, M95256_SPI_CS);

   return 0;
}

int m95256_write(uint16_t addr, uint8_t *buf, int count)
{
   uint8_t cmd[] = {M95256_CMD_WRITE, (addr >> 8) & 0xFF, addr & 0xFF};

   // Write enable
   hal_spi_select(M95256_SPI, M95256_SPI_CS);
   hal_spi_transmit(M95256_SPI, M95256_CMD_WREN);
   hal_spi_deselect(M95256_SPI, M95256_SPI_CS);

   hal_spi_select(M95256_SPI, M95256_SPI_CS);
   hal_spi_write(M95256_SPI, cmd, sizeof(cmd));
   hal_spi_write(M95256_SPI, buf, count);
   hal_spi_deselect(M95256_SPI, M95256_SPI_CS);

   while(m95256_get_status() & 0x1);

   return 0;
}


int m95256_erase_chip(void)
{
   uint8_t buf[64];
   int addr;

   memset(buf, 0xFF, sizeof(buf));
   for (addr = 0; addr < M95256_SIZE; addr += sizeof(buf))
   {
      m95256_write(addr, buf, sizeof(buf));
   }

   TRACE("Chip erased");

   return 0;
}


static uint8_t m95256_get_status(void)
{
   uint8_t res;

   hal_spi_select(M95256_SPI, M95256_SPI_CS);
   hal_spi_transmit(M95256_SPI, M95256_CMD_RDSR);
   res = hal_spi_transmit(M95256_SPI, 0xFF);
   hal_spi_deselect(M95256_SPI, M95256_SPI_CS);

   return res;
}
