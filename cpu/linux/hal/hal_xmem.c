
#include "system.h"

TRACE_TAG(hal_xmem);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_HAL_XMEM_SIZE
#define CFG_HAL_XMEM_SIZE     8192
#endif

#ifndef CFG_HAL_XMEM_FILENAME
#define CFG_HAL_XMEM_FILENAME    "xmem.bin"
#endif

// Locals:
static FILE *fm = NULL;

int hal_xmem_init(void)
{
   // Try open the file 
   if ((fm = fopen(CFG_HAL_XMEM_FILENAME, "r+")) == NULL) 
   {
      // File does not exist, create new
      if ((fm = fopen(CFG_HAL_XMEM_FILENAME, "w+")) == NULL) 
      {
         TRACE_ERROR("Can not open xmem file %s", CFG_HAL_XMEM_FILENAME);
         return -1;
      }
   }

   // Test xmem file size
   if (ftruncate(fileno(fm), CFG_HAL_XMEM_SIZE) < 0)
   {
      TRACE_ERROR("Resize xmem file failed");
      fclose(fm);
      return -1;
   }

   return 0;
}

int hal_xmem_read(uint32_t offset, void *buf, int nbytes)
{
   ASSERT(fm != NULL);

   if (fseek(fm, offset, SEEK_SET) < 0)
   {
      TRACE_ERROR("Seek xmem file to offset: %d failed", offset);
      return -1;
   }

   return fread(buf, sizeof(char), nbytes, fm);
}

int hal_xmem_write(uint32_t offset, const void *buf, int nbytes)
{
   int res;

   ASSERT(fm != NULL);

   if (fseek(fm, offset, SEEK_SET) < 0)
   {
      TRACE_ERROR("Seek xmem file to offset: %d failed", offset);
      return -1;
   }

   res = fwrite(buf, sizeof(char), nbytes, fm);
   fflush(fm);

   return res;
}

int hal_xmem_erase(uint32_t offset, uint32_t nbytes)
{
   ASSERT(fm != NULL);

   fclose(fm);

   if ((fm = fopen(CFG_HAL_XMEM_FILENAME, "w+")) == NULL) 
   {
      TRACE_ERROR("Can not open xmem file %s", CFG_HAL_XMEM_FILENAME);
      return -1;
   }

   if (ftruncate(fileno(fm), CFG_HAL_XMEM_SIZE) < 0)
   {
      fclose(fm);
      TRACE_ERROR("Resize xmem file failed");
      return -1;
   }

   TRACE("hal_xmem_erase  offset: %d  len: %d", offset, nbytes);

   return 0;
}
