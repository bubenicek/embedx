/**
 * \file dfs_uffs.c    \brief Data file system suported UFFS flash filesystem
 */

#include <string.h>

#include "system.h"
#include "uffs/uffs_fd.h"
#include "uffs_interface.h"

#include "dfs.h"
#include "crc32.h"

#define TRACE_TAG "dfs-uffs"
#if !ENABLE_TRACE_DFS
#undef TRACE
#define TRACE(...)
#endif

#define CFG_FW_UPGRADE_FILENAME        "/fw_upgrade.bin"
#define CFG_FW_BACKUP_FILENAME         "/fw_backup.bin"

typedef struct
{
   int fd;
   uint32_t offset;
   const char *filename;

} dfs_desc_t;

// Locals:
static const uint32_t active_sectors[] = CFG_DFS_ACTIVE_SECTORS;
#define NUM_ACTIVE_SECTORS (sizeof(active_sectors) / sizeof(uint32_t))

static dfs_desc_t dfs_desc[] = 
{
   {-1, 0, NULL},
   {-1, 0, CFG_FW_UPGRADE_FILENAME},
   {-1, 0, CFG_FW_BACKUP_FILENAME}
};
#define DFS_NUM_DESC  (sizeof(dfs_desc) / sizeof(dfs_desc_t))


/** Initialize dfs */
int dfs_init(void)
{
   // Initialize filesystem
   if (uffs_init_filesystem() != 0)
   {
      TRACE_ERROR("Init UFFS");
      return -1;
   }

   // Initialize access to internal flash
   if (hal_flash_init() != 0)
      return -1;

   return 0;
}

int dfs_deinit(void)
{
   hal_flash_deinit();
   VERIFY(uffs_deinit_filesystem() == 0);
   return 0;
}

/** Create dfs (erase) */
int dfs_create(dfs_type_t dfs_type)
{
   dfs_desc_t *desc;
   
   ASSERT(dfs_type < DFS_NUM_DESC);
   desc = &dfs_desc[dfs_type];

   if (desc->filename != NULL)
   {
      if ((desc->fd = uffs_open(desc->filename, UO_CREATE|UO_TRUNC|UO_WRONLY, 0)) < 0)
      {
         TRACE_ERROR("Create file %s", desc->filename);
         return -1;
      }
      
      return 0;
   }
   else
   {
      return dfs_erase(dfs_type);
   }
}

/** Open dfs */
int dfs_open(dfs_type_t dfs_type, app_header_t *hdr)
{
   dfs_desc_t *desc;
   
   ASSERT(dfs_type < DFS_NUM_DESC);
   desc = &dfs_desc[dfs_type];

   memset(hdr, 0, sizeof(app_header_t));

   if (desc->filename != NULL)
   {
      if ((desc->fd = uffs_open(desc->filename, UO_RDWR, 0)) < 0)
      {
         //TRACE_ERROR("Open file %s", filename);
         goto fail;
      }

      TRACE("Open file: %s", desc->filename);

      // Read and validate header
      if (dfs_read_header(dfs_type, hdr) != 0)
      {
         TRACE_ERROR("Read header");
         goto fail;
      }

      // Seek to begin of file
      if (uffs_seek(desc->fd, 0, USEEK_SET) < 0)
      {
         TRACE_ERROR("Seek to begin of file");
         goto fail;
      }
      
      desc->offset = 0;
   }
   else
   {
      TRACE("Open active dfs");

      // Set start address of flash
      desc->offset = CFG_DFS_ACTIVE_START_ADDR;

      // Read and validate header
      if (dfs_read_header(dfs_type, hdr) != 0)
      {
         TRACE_ERROR("Read header");
         goto fail;
      }

      // Seek to begin of flash
      desc->offset = CFG_DFS_ACTIVE_START_ADDR;
   }

   return 0;

fail:
   dfs_close(dfs_type);
   return -1;
}

/** Close dfs */
int dfs_close(dfs_type_t dfs_type)
{
   dfs_desc_t *desc;
   
   ASSERT(dfs_type < DFS_NUM_DESC);
   desc = &dfs_desc[dfs_type];

   if (desc->filename != NULL)
   {
      if (desc->fd != -1)
      {
         uffs_close(desc->fd);
         desc->fd = -1;
      }
   }

   return 0;
}

/** Read from dfs */
int dfs_read(dfs_type_t dfs_type, void *buf, int bufsize)
{
   int res;
   dfs_desc_t *desc;
   
   ASSERT(dfs_type < DFS_NUM_DESC);
   desc = &dfs_desc[dfs_type];

   if (desc->filename != NULL)
      res = uffs_read(desc->fd, buf, bufsize);
   else
      res = hal_flash_read(desc->offset, buf, bufsize);
   
   if (res > 0)
      desc->offset += res;

   return res;
}

/** Write to dfs */
int dfs_write(dfs_type_t dfs_type, const void *buf, int bufsize)
{
   int res;
   dfs_desc_t *desc;
   
   ASSERT(dfs_type < DFS_NUM_DESC);
   desc = &dfs_desc[dfs_type];

   if (desc->filename != NULL)
      res = uffs_write(desc->fd, buf, bufsize);
   else
      res = hal_flash_write(desc->offset, buf, bufsize);

   if (res > 0)
      desc->offset += res;

   return res;
}

/** Erase dfs */
int dfs_erase(dfs_type_t dfs_type)
{
   dfs_desc_t *desc;
   
   ASSERT(dfs_type < DFS_NUM_DESC);
   desc = &dfs_desc[dfs_type];

   if (desc->filename != NULL)
   {
      return uffs_remove(desc->filename);
   }
   else
   {
      int ix;

      // Erase active storage
      for (ix = 0; ix < NUM_ACTIVE_SECTORS; ix++)
      {
         hal_flash_erase(active_sectors[ix]);

         hal_wdg_reset();

         hal_led_set(LED_DF, 0);
         hal_led_set(LED_ZWAVE, 0);
         hal_led_set(LED_SYSTEM, 0);

         hal_delay_ms(50);

         hal_led_set(LED_DF, 1);
         hal_led_set(LED_ZWAVE, 1);
         hal_led_set(LED_SYSTEM, 1);
      }

      desc->offset = CFG_DFS_ACTIVE_START_ADDR;

      return 0;
   }
}
