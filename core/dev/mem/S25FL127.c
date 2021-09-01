
#include "system.h"
#include "S25FL127.h"

#define TRACE_TAG    "S25FL127"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#undef TRACE_PRINTFF
#undef TRACE_PRINTF
#define TRACE(...)
#define TRACE_PRINTFF(...)
#define TRACE_PRINTF(...)
#endif

#define MANUFACTURER_ID       0x01
#define DEVICE_ID_MSB         0x20
#define DEVICE_ID_LSB         0x18

#define STAT_WIP               1
#define STAT_WEL               2

#define CMD_WRITE_STATUS_REG   0x01
#define CMD_PAGE_PROGRAM       0x02
#define CMD_READ_DATA          0x03
#define CMD_WRITE_DISABLE      0x04
#define CMD_READ_STATUS_REG    0x05
#define CMD_WRITE_ENABLE       0x06
#define CMD_READ_HIGH_SPEED    0x0B
#define CMD_SECTOR_ERASE       0x20
#define CMD_BLOCK32K_ERASE     0x52
#define CMD_RESET_DEVICE       0xF0
#define CMD_READ_ID            0x9F
#define CMD_RELEASE_POWER_DOWN 0xAB
#define CMD_POWER_DOWN         0xB9
#define CMD_CHIP_ERASE         0xC7
#define CMD_BLOCK64K_ERASE     0xD8

#define S25FL127_NUM_PAGES_PER_SECTOR     (S25FL127_SECTOR_SIZE / S25FL127_PAGE_SIZE)
#define S25FL127_NUM_PAGES                (S25FL127_NUM_PAGES_PER_SECTOR * S25FL127_NUM_SECTORS)

#define S25FL127_TEST          0

// Prototypes:
static uint8_t s25fl127_read_status(void);
static void s25fl127_busy_wait(void);


int s25fl127_init(void)
{
   static uint8_t initialized = 0;
   uint8_t id[3];
   volatile int tmo;

   if (!initialized)
   {
#ifdef DF_SPI_GPIO_RESET      
      // Reset chip
      hal_gpio_set(DF_SPI_GPIO_RESET, 0);
      for (tmo = 0; tmo < 10000; tmo++);
      hal_gpio_set(DF_SPI_GPIO_RESET, 1);
      for (tmo = 0; tmo < 10000; tmo++);
#endif
      s25fl127_busy_wait();

      hal_spi_select(DF_SPI, DF_SPI_CS);
      hal_spi_transmit(DF_SPI, CMD_READ_ID);
      hal_spi_read(DF_SPI, id, sizeof(id));
      hal_spi_deselect(DF_SPI, DF_SPI_CS);

      if (!(id[0] == MANUFACTURER_ID && id[1] == DEVICE_ID_MSB && id[2] == DEVICE_ID_LSB))
      {
         TRACE_ERROR("S25Fl127 not found");
         return -1;
      }

      initialized = 1;
      TRACE("S25FL127 init, ID: 0x%X%X%X", id[0], id[1], id[2]);

#if S25FL127_TEST == 1
      extern int s25fl127_test(void);
      while(s25fl127_test() == 0);
      ASSERT(0);
#endif
   }

   return 0;
}

int s25fl127_write_page(uint32_t page, uint8_t *pagebuf)
{
   uint32_t address = page * S25FL127_PAGE_SIZE;

   s25fl127_busy_wait();

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_WRITE_ENABLE);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_PAGE_PROGRAM);
   hal_spi_transmit(DF_SPI, (address >> 16) & 0xff);
   hal_spi_transmit(DF_SPI, (address >> 8) & 0xff);
   hal_spi_transmit(DF_SPI, address & 0xff);
   hal_spi_write(DF_SPI, pagebuf, S25FL127_PAGE_SIZE);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   return 0;
}

int s25fl127_read_page(uint32_t page, uint8_t *pagebuf)
{
   uint32_t address = page * S25FL127_PAGE_SIZE;

   s25fl127_busy_wait();

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_READ_DATA);
   hal_spi_transmit(DF_SPI, (address >> 16) & 0xff);
   hal_spi_transmit(DF_SPI, (address >> 8) & 0xff);
   hal_spi_transmit(DF_SPI, address & 0xff);
   hal_spi_read(DF_SPI, pagebuf, S25FL127_PAGE_SIZE);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   return 0;
}

int s25fl127_read(uint32_t addr, void *buf, int nbytes)
{
   s25fl127_busy_wait();

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_READ_DATA);
   hal_spi_transmit(DF_SPI, (addr >> 16) & 0xff);
   hal_spi_transmit(DF_SPI, (addr >> 8) & 0xff);
   hal_spi_transmit(DF_SPI, addr & 0xff);
   hal_spi_read(DF_SPI, buf, nbytes);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   return nbytes;
}

int s25fl127_write(uint32_t addr, void *buf, int size)
{
   int total = 0;
   uint32_t local_addr = addr;
   uint8_t *p_buf = buf;
   uint32_t write_size = size;
   uint32_t write_bytes;

   write_bytes = S25FL127_PAGE_SIZE - ((local_addr) & (S25FL127_PAGE_SIZE - 1));
   if (write_bytes > write_size)
      write_bytes = write_size;

   while (write_size > 0)
   {
      s25fl127_busy_wait();

      hal_spi_select(DF_SPI, DF_SPI_CS);
      hal_spi_transmit(DF_SPI, CMD_WRITE_ENABLE);
      hal_spi_deselect(DF_SPI, DF_SPI_CS);

      hal_spi_select(DF_SPI, DF_SPI_CS);
      hal_spi_transmit(DF_SPI, CMD_PAGE_PROGRAM);
      hal_spi_transmit(DF_SPI, (local_addr >> 16) & 0xff);
      hal_spi_transmit(DF_SPI, (local_addr >> 8) & 0xff);
      hal_spi_transmit(DF_SPI, local_addr & 0xff);
      hal_spi_write(DF_SPI, p_buf, write_bytes);
      hal_spi_deselect(DF_SPI, DF_SPI_CS);

      // Update counters and data pointers for the next page 
      total += write_bytes;
      write_size -= write_bytes;
      p_buf += write_bytes;
      local_addr += write_bytes;
      write_bytes = (write_size > S25FL127_PAGE_SIZE) ? S25FL127_PAGE_SIZE : write_size;
   }

   return total;
}

/** Erase the sector which contains the specified */
int s25fl127_erase_sector(uint32_t sector)
{
   uint32_t address = sector * S25FL127_SECTOR_SIZE;

   s25fl127_busy_wait();

   // Send Write Enable command
   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_WRITE_ENABLE);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_BLOCK64K_ERASE);
   hal_spi_transmit(DF_SPI, (address >> 16) & 0xff);
   hal_spi_transmit(DF_SPI, (address >> 8) & 0xff);
   hal_spi_transmit(DF_SPI, address & 0xff);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   return 0;
}

int s25fl127_erase_chip(void)
{
   s25fl127_busy_wait();

   TRACE_PRINTFF("Erasing chip ... ");

   // Send Write Enable command
   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_WRITE_ENABLE);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_CHIP_ERASE);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   s25fl127_busy_wait();

   TRACE_PRINTF("done\r\n");

   return 0;
}


static uint8_t s25fl127_read_status(void)
{
   uint8_t c;

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, CMD_READ_STATUS_REG);
   c = hal_spi_transmit(DF_SPI, 0x00);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   return(c);
}

static void s25fl127_busy_wait(void)
{
   while(s25fl127_read_status() & STAT_WIP);
}



#if S25FL127_TEST == 1
int s25fl127_test(void)
{
   int ix, ip, ib, addr;
   static uint8_t buf[S25FL127_PAGE_SIZE];
   uint32_t seed;

   //
   // Fill data to all pages
   //
   s25fl127_erase_chip();

   srand(0);
   TRACE_PRINTFF("Filing pages ... ");
   for (ip = 0; ip < S25FL127_NUM_PAGES; ip++)
   {
      for (ix = 0; ix < S25FL127_PAGE_SIZE; ix++)
      {
         buf[ix] = rand() % 256;
      }

      s25fl127_write_page(ip, buf);
   }
   TRACE_PRINTF("done\r\n");

   srand(0);
   TRACE_PRINTFF("Comparing pages ... ");
   for (ip = 0; ip < S25FL127_NUM_PAGES; ip++)
   {
      s25fl127_read_page(ip, buf);

      for (ix = 0; ix < S25FL127_PAGE_SIZE; ix++)
      {
         if (buf[ix] != rand() % 256)
         {
            TRACE_ERROR("Compare data  page: %d  offset: %d   0x%X != 0x%X", ip, ix, buf[ix], ix);
            return -1;
         }
      }
   }
   TRACE_PRINTF("done\r\n");

   //
   // Write pages in blocks
   //
   seed = hal_time_ms();
   srand(seed);
   for (ib = 0; ib < S25FL127_NUM_SECTORS; ib++)
   {
      TRACE_PRINTFF("Writing pages in sector: %d ... ", ib);

      s25fl127_erase_sector(ib);

      for (ip = 0; ip < S25FL127_NUM_PAGES_PER_SECTOR; ip++)
      {
         for (ix = 0; ix < S25FL127_PAGE_SIZE; ix++)
         {
            buf[ix] = rand() % 256;
         }

         addr = (ib * S25FL127_NUM_PAGES_PER_SECTOR) + ip;

         s25fl127_write_page(addr, buf);
      }
      TRACE_PRINTF("done\r\n");
   }

   //
   // Read data from pages in blocks
   //
   srand(seed);
   for (ib = 0; ib < S25FL127_NUM_SECTORS; ib++)
   {
      TRACE_PRINTFF("Reading pages in sector: %d ... ", ib);

      for (ip = 0; ip < S25FL127_NUM_PAGES_PER_SECTOR; ip++)
      {
         addr = (ib * S25FL127_NUM_PAGES_PER_SECTOR) + ip;
         s25fl127_read_page(addr, buf);

         for (ix = 0; ix < S25FL127_PAGE_SIZE; ix++)
         {
            if (buf[ix] != rand() % 256)
            {
               TRACE_ERROR("Compare data  block: %d  page: %d  offset: %d", ib, ip, ix);
               return -1;
            }
         }
      }
      TRACE_PRINTF("done\r\n");
   }

   TRACE("Test OK");

   return 0;
}
#endif

