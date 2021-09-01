/*
# internal flash content (for STM32F407VG, 128 kB SRAM, 64 kB CCM RAM, 1024 kB FLASH)
#
# +--------------------------+---------------------+  0x080FFFFF - end of the internal flash
# |   Sector 11, 128 Kbyte   |                     |
# |   Sector 10, 128 Kbyte   |                     |
# |   Sector 9, 128 Kbyte    |                     |
# |   Sector 8, 128 Kbyte    |                     |
# |   Sector 7, 128 Kbyte    |                     |
# |   Sector 6, 128 Kbyte    |  main application   |
# |   Sector 5, 128 Kbyte    |       section       |
# |   Sector 4, 64 Kbyte     |                     | 0x08010000 - beginning of Sector 4
# +------------------------------------------------|
# |   Sector 3, 16 Kbyte     |                     |
# |   Sector 2, 16 Kbyte     |                     |
# |   Sector 1, 16 Kbyte     |      bootloader     |
# |   Sector 0, 16 Kbyte     |       section       |
# +--------------------------+---------------------+  0x08000000 - beginning of the internal flash
*/

#include <string.h>
#include "system.h"

TRACE_TAG(hal_flash);
#if !ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

#define FLASH_LOCK() do { \
   FLASH_Lock(); \
} while(0)

#define FLASH_UNLOCK() do { \
   FLASH_Unlock(); \
   FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); \
} while(0)


int hal_flash_init(void)
{
   return 0;
}

int hal_flash_deinit(void)
{
   return 0;
}

int hal_flash_erase(uint32_t start_addr, int length)
{
   /*
   TRACE("Erase sector: 0x%X", sector);

   FLASH_UNLOCK();

   if (FLASH_EraseSector(sector, FLASH_COMPLETE) != FLASH_COMPLETE)
   {
      FLASH_LOCK();
      TRACE_ERROR("Flash erase sector 0x%X", sector);
      return -1;
   }

   FLASH_LOCK();
   */

   // TODO: 

   return -1;
}

int hal_flash_read(uint32_t addr, void *buf, int bufsize)
{
   memcpy(buf, (uint8_t *)addr, bufsize);
   return bufsize;
}

int hal_flash_write(uint32_t addr, const void *buf, int bufsize)
{
   int ix;
   const uint8_t *pbuf = buf;

   FLASH_UNLOCK();

   for (ix = 0; ix < bufsize; ix++)
   {
      if (FLASH_ProgramByte(addr + ix, pbuf[ix]) != FLASH_COMPLETE)
      {
         FLASH_LOCK();
         TRACE_ERROR("Program byte at addr: 0x%X failed", addr);
         return -1;
      }
   }

   FLASH_LOCK();

   return bufsize;
}

