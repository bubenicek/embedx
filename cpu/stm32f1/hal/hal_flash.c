
#include "system.h"

TRACE_TAG(hal_flash);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

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
   uint32_t PAGEError = 0;

   HAL_FLASH_Unlock();

   FLASH_EraseInitTypeDef EraseInitStruct;
   EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
   EraseInitStruct.PageAddress = start_addr;
   EraseInitStruct.NbPages     = 1; // TODO: ((start_addr + length) - start_addr) / FLASH_PAGE_SIZE;
   if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
   {
      TRACE_ERROR("HAL_FLASHEx_Erase failed");
      return -1;
   }

   return 0;
}

int hal_flash_read(uint32_t addr, void *buf, int bufsize)
{
   memcpy(buf, (uint8_t *)addr, bufsize);
   return bufsize;
}

int hal_flash_write(uint32_t addr, const void *buf, int bufsize)
{
   uint32_t *pbuf = (uint32_t *)buf;
   uint32_t addr_end = addr + bufsize;

   while (addr < addr_end)
   {
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *pbuf) != HAL_OK)
      {
         TRACE_ERROR("HAL_FLASH_Program failed");
         return -1;
      }

      addr += 4;
      pbuf++;
   }

   return bufsize;
}
