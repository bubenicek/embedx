
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "system.h"
#include "at45db.h"

#define TRACE_TAG    "at45db"

#define AT45DB_TEST                 0

#define AT45DB_RESET_GPIO_CLK       RCC_AHB1Periph_GPIOC
#define AT45DB_RESET_GPIO_PORT      GPIOC
#define AT45DB_RESET_PIN            GPIO_Pin_9

#define AT45DB_PAGE_PROGRAM_1		   0x88
#define AT45DB_PAGE_READ			   0xD2
#define AT45DB_BLOCK_ERASE			   0x50
#define AT45DB_PAGE_ERASE			   0x81
#define AT45DB_BUFFER_1_WR				0x84
#define AT45DB_BUFFER_1_TO_PAGE		0x88     // without auto erase


// Prototypes:
static uint8_t at45db_read_status(void);
static void at45db_busy_wait(void);

int at45db_init(void)
{
   int i = 0, id = 0;
   GPIO_InitTypeDef GPIO_InitStructure;

   RCC_AHB1PeriphClockCmd(AT45DB_RESET_GPIO_CLK, ENABLE);

   // Configure Reset PIN - LOW
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_Pin =  AT45DB_RESET_PIN;
   GPIO_Init(AT45DB_RESET_GPIO_PORT, &GPIO_InitStructure);

   // Reset DF
   GPIO_ResetBits(AT45DB_RESET_GPIO_PORT, AT45DB_RESET_PIN);
   hal_delay_ms(1);
   GPIO_SetBits(AT45DB_RESET_GPIO_PORT, AT45DB_RESET_PIN);
   hal_delay_ms(1);

   while (id != 0x1F)
   {
      hal_spi_select(DF_SPI, DF_SPI_CS);
      hal_spi_transmit(DF_SPI, 0x9F);
      id = hal_spi_transmit(DF_SPI, 0x00);
      hal_spi_deselect(DF_SPI, DF_SPI_CS);
      hal_delay_ms(10);
      if (i++ > 10)
      {
         TRACE_ERROR("AT45DB not found");
         return -1;
      }
   }

#if AT45DB_TEST == 1
static int at45db_test(void);
   at45db_test();
   ASSERT(0);
#endif

   return 0;
}

int at45db_read_page(uint16_t page, uint8_t *buffer)
{
   uint8_t cmd[8] = {AT45DB_PAGE_READ,
                     (page >> 7) & 0xFF,
                     (page << 1) & 0xFF,
                     0,
                     0,
                     0,
                     0,
                     0,
                    };

   at45db_busy_wait();
   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_write(DF_SPI, cmd, 8);
   hal_spi_read(DF_SPI, buffer, AT45DB_PAGE_SIZE);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   return 0;
}

int at45db_write_page(uint16_t page, uint8_t *buffer)
{
   uint8_t cmd[4] = {AT45DB_BUFFER_1_WR,
                     0,
                     0,
                     0
                    };

   uint8_t cmd2[4] = {AT45DB_BUFFER_1_TO_PAGE,
                     (page >> 7) & 0xFF,
                     (page << 1) & 0xFF,
                     0
                     };

   // Write page to buffer
   at45db_busy_wait();
   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_write(DF_SPI, cmd, 4);
   hal_spi_write(DF_SPI, buffer, AT45DB_PAGE_SIZE);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   // Write buffer to page without erase
   at45db_busy_wait();
   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_write(DF_SPI, cmd2, 4);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   return 0;
}

int at45db_erase_chip(void)
{
   uint8_t cmd[4] = {0xC7, 0x94, 0x80, 0x9A};

   TRACE("Erasing chip ...");

   at45db_busy_wait();

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_write(DF_SPI, cmd, 4);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   // wait until AT45DB161 is ready again
   at45db_busy_wait();

   TRACE("Erase done");

   return 0;
}

int at45db_erase_block(uint16_t addr)
{
   uint8_t cmd[4] = {AT45DB_BLOCK_ERASE,
                     (addr >> 4) & 0xFF,
                     (addr << 4) & 0xFF,
                     0
                    };

   at45db_busy_wait();

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_write(DF_SPI, cmd, 4);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   at45db_busy_wait();

   return 0;
}

int at45db_erase_page(uint16_t addr)
{
   uint8_t cmd[4] = {AT45DB_PAGE_ERASE,
                     (addr >> 7) & 0xFF,
                     (addr << 1) & 0xFF,
                     0
                    };

   at45db_busy_wait();

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_write(DF_SPI, cmd, 4);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   // wait until AT45DB161 is ready again
   at45db_busy_wait();

   return 0;
}

static uint8_t at45db_read_status(void)
{
   uint8_t retval;

   hal_spi_select(DF_SPI, DF_SPI_CS);
   hal_spi_transmit(DF_SPI, 0xD7);
   retval = hal_spi_transmit(DF_SPI, 0xFF);
   hal_spi_deselect(DF_SPI, DF_SPI_CS);

   return retval;
}

static void at45db_busy_wait(void)
{
   while(!(at45db_read_status() & 0x80));
}

#if AT45DB_TEST == 1
static int at45db_test(void)
{
   int ix, ip, ib, addr;
   static uint8_t buf[AT45DB_PAGE_SIZE];

   //
   // Fill data to all pages
   //
   at45db_erase_chip();

   srand(0);
   TRACE_PRINTFF("Filing pages ... ");
   for (ip = 0; ip < AT45DB_NUM_PAGES; ip++)
   {
      for (ix = 0; ix < AT45DB_PAGE_SIZE; ix++)
      {
         buf[ix] = rand() % 256;
      }

      at45db_write_page(ip, buf);
   }
   TRACE_PRINTF("done\r\n");

   srand(0);
   TRACE_PRINTFF("Comparing pages ... ");
   for (ip = 0; ip < AT45DB_NUM_PAGES; ip++)
   {
      at45db_read_page(ip, buf);

      for (ix = 0; ix < AT45DB_PAGE_SIZE; ix++)
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
   srand(0xff);
   for (ib = 0; ib < AT45DB_NUM_BLOCKS; ib++)
   {
      TRACE_PRINTFF("Writing pages in block: %d ... ", ib);

      at45db_erase_block(ib);

      for (ip = 0; ip < AT45DB_PAGES_PER_BLOCK; ip++)
      {
         for (ix = 0; ix < AT45DB_PAGE_SIZE; ix++)
         {
            buf[ix] = rand() % 256;
         }

         addr = (ib * AT45DB_PAGES_PER_BLOCK) + ip;

         at45db_write_page(addr, buf);
      }
      TRACE_PRINTF("done\r\n");
   }

   //
   // Read data from pages in blocks
   //
   srand(0xff);
   for (ib = 0; ib < AT45DB_NUM_BLOCKS; ib++)
   {
      TRACE_PRINTFF("Reading pages in block: %d ... ", ib);

      for (ip = 0; ip < AT45DB_PAGES_PER_BLOCK; ip++)
      {
         addr = (ib * AT45DB_PAGES_PER_BLOCK) + ip;
         at45db_read_page(addr, buf);

         for (ix = 0; ix < AT45DB_PAGE_SIZE; ix++)
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
#endif  // AT45DB_TEST
