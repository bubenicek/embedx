
#include "system.h"
#include "dfs.h"
#include "crc32.h"

#define TRACE_TAG "dfs"
#if !ENABLE_TRACE_DFS
#undef TRACE
#undef TRACE_ERROR
#define TRACE(...)
#define TRACE_ERROR(...)
#endif

static uint8_t buffer[1024];

/** Read header from dfs and check CRC */
int dfs_read_header(dfs_type_t dfs_type, app_header_t *hdr)
{
   int size, len;
   uint32_t crc, tmpcrc;

   // Count CRC before header
   for (crc = 0, size = 0; size < CFG_APP_HEADER_OFFSET; size++)
   {
      if (dfs_read(dfs_type, buffer, 1) != 1)
      {
         TRACE_ERROR("dfs(%d) read data", dfs_type);
         return -1;
      }
      
      crc = crc32(crc, buffer, 1);
   }

   // Read header
   if (dfs_read(dfs_type, hdr, sizeof(app_header_t)) != sizeof(app_header_t))
   {
      TRACE_ERROR("dfs(%d) read header", dfs_type);
      return -1;
   }
   size += sizeof(app_header_t);
   
   if (hdr->magic != APP_HEADER_MAGIC)
   {
      TRACE_ERROR("dfs(%d) bad magic", dfs_type);
      return -1;
   }

   TRACE("App ver. %d.%d  size: %d  CRC: 0x%X",
      APP_VERSION_MAJOR(hdr->fw_version), APP_VERSION_MINOR(hdr->fw_version), hdr->fw_size, hdr->fw_crc);

   // Count crc with 0
   tmpcrc = hdr->fw_crc;
   hdr->fw_crc = 0;
   crc = crc32(crc, (uint8_t *)hdr, sizeof(app_header_t));
   hdr->fw_crc = tmpcrc;

   // Count crc after header
   while (size < hdr->fw_size)
   {
      len = hdr->fw_size - size > sizeof(buffer) ? sizeof(buffer) : hdr->fw_size - size;
      if (dfs_read(dfs_type, buffer, len) != len)
      {
         TRACE_ERROR("dfs(%d) read data", dfs_type);
         return -1;
      }

      crc = crc32(crc, buffer, len);
      size += len;
   }

   if (crc != hdr->fw_crc)
   {
      TRACE_ERROR("dfs(%d) bad crc", dfs_type);
      return -1;
   }

   TRACE("dfs(%d) verify OK", dfs_type);

   return 0;
}

/** Copy data between dfs */
int dfs_copy(dfs_type_t src, dfs_type_t dst)
{
   int len, size, counter = 0;
   app_header_t src_hdr;
   
   if (dfs_open(src, &src_hdr) != 0)
   {
      TRACE_ERROR("Open dfs(%d)", dst);
      goto fail;
   }
      
   if (dfs_erase(dst) != 0)
   {
      TRACE_ERROR("Erase dfs(%d)", dst);      
      return -1;
   }
      
   TRACE("Copy dfs(%d) -> dfs(%d)  size: %d", src, dst,  src_hdr.fw_size);

   // Write upgrade file to active file
   for (size = src_hdr.fw_size; size > 0; size -= len)
   {      
      len = size > sizeof(buffer) ? sizeof(buffer) : size;      
      
      if (dfs_read(src, buffer, len) != len)
      {
         TRACE_ERROR("Read from dfs(%d)", src);
         goto fail;
      }

      if (dfs_write(dst, buffer, len) != len)
      {
         TRACE_ERROR("Write to dfs(%d)", dst);
         goto fail;
      }  

      if (++counter == 32) 
      {
         counter = 0;
         hal_led_toggle(LED_DF);
         hal_led_toggle(LED_ZWAVE);
         hal_led_toggle(LED_SYSTEM);
      }

      hal_wdg_reset();
   }

   TRACE("Copy done");

   dfs_close(src);
   dfs_close(dst);

   return 0;

fail:
   dfs_close(src);
   dfs_close(dst);
   return -1;
}
