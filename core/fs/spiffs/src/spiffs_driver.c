
#include "spiffs.h"
#include "spiffs_nucleus.h"
#include "dev/mem/S25FL127.h"

#define TRACE_TAG "spiffs_drv"
#if !ENABLE_TRACE_SPIFS_DRV
#undef TRACE
#define TRACE(...)
#endif

#define TEST_SPIFFS                    0

//
// SPI dataflsh interface
//
#define df_init()                      s25fl127_init()
#define df_read(_addr, _buf, _count)   s25fl127_read(_addr, _buf, _count)
#define df_write(_addr, _buf, _count)  s25fl127_write(_addr, _buf, _count)
#define df_erase_block(_block)         s25fl127_erase_sector(_block)
#define df_erase_chip()                s25fl127_erase_chip()


#define SPIFFS_FLASH_START             0
#define SPIFFS_NUM_BLOCKS              S25FL127_NUM_SECTORS
#define SPIFFS_PAGE_SIZE               S25FL127_PAGE_SIZE
#define SPIFFS_BLOCK_SIZE              S25FL127_SECTOR_SIZE
#define SPIFFS_FLASH_SIZE              (SPIFFS_NUM_BLOCKS * SPIFFS_BLOCK_SIZE)

#define WORK_BUF_SIZE                  (SPIFFS_PAGE_SIZE * 2)
#define FD_BUF_SIZE                    (32 * 2)
#define CACHE_BUF_SIZE                 (SPIFFS_PAGE_SIZE + 32)


// Prototypes:
static s32_t spiffs_drv_read(u32_t addr, u32_t size, u8_t *dst);
static s32_t spiffs_drv_write(u32_t addr, u32_t size, u8_t *src);
static s32_t spiffs_drv_erase(u32_t addr, u32_t size);

// Locals:
static spiffs fs;
static u8_t spiffs_work_buf[WORK_BUF_SIZE];
static u8_t spiffs_fds[FD_BUF_SIZE];
#if SPIFFS_CACHE
static u8_t spiffs_cache_buf[CACHE_BUF_SIZE];
#endif

/** Initialize (mount) filesystem */
int spiffs_mount(spiffs **pfs)
{
   spiffs_config cfg;
   uint32_t total, used;
   
   if (df_init() != 0)
   {
      TRACE_ERROR("Init dataflash");
      return -1;
   }
      
   cfg.phys_size = SPIFFS_FLASH_SIZE;
   cfg.phys_addr = SPIFFS_FLASH_START;  
   cfg.phys_erase_block = SPIFFS_BLOCK_SIZE;
   cfg.log_block_size = SPIFFS_BLOCK_SIZE;
   cfg.log_page_size = SPIFFS_PAGE_SIZE;

   cfg.hal_read_f = spiffs_drv_read;
   cfg.hal_write_f = spiffs_drv_write;
   cfg.hal_erase_f = spiffs_drv_erase;

   if (SPIFFS_mount(&fs,
                    &cfg,
                    spiffs_work_buf,
                    spiffs_fds,
                    sizeof(spiffs_fds),
#if SPIFFS_CACHE                   
                    spiffs_cache_buf,
                    sizeof(spiffs_cache_buf),
#else
                    NULL, 0,
#endif                    
                    0) != 0)
   {
      TRACE_ERROR("SPIFFS_mount");
      return -1;
   }

   TRACE("SPIFFS mounted");   
  
   SPIFFS_info(&fs, &total, &used);
   TRACE("Phys size:                   %u", cfg.phys_size);
   TRACE("Memory buffers size:         %u", sizeof(fs) + sizeof(spiffs_work_buf) + sizeof(spiffs_fds));
   TRACE("Page index byte len:         %u", SPIFFS_CFG_LOG_PAGE_SZ(&fs));
   TRACE("Object lookup pages:         %u", SPIFFS_OBJ_LOOKUP_PAGES(&fs));
   TRACE("Page pages per block:        %u", SPIFFS_PAGES_PER_BLOCK(&fs));
   TRACE("Object header index entries: %u", SPIFFS_OBJ_HDR_IX_LEN(&fs));
   TRACE("Object index entries:        %u", SPIFFS_OBJ_IX_LEN(&fs));
   TRACE("Available file descriptors:  %u", fs.fd_count);
   TRACE("Free blocks:                 %u", fs.free_blocks);
   TRACE("Total space size:            %u", total);
   TRACE("Used space size:             %u", used);
   TRACE("Free space size:             %u", total - used);

   *pfs = &fs;
   
#if TEST_SPIFFS   
   extern int spiffs_test(int fileindex, int filesize);  
   while(1)
   {
      ASSERT(spiffs_test(0, 4 * 1024 * 1024) == 0);
//      ASSERT(spiffs_test(0, 10 * 1024) == 0);
   }
#else
   return 0;
#endif   
}

/** Unmount filesystem */
int spiffs_unmount(spiffs *fs)
{
   SPIFFS_unmount(fs);

   TRACE("SPIFFS unmounted");

   return 0;
}

/** Erase filesystem */
int spiffs_format(spiffs *fs)
{
   if (SPIFFS_format(fs) != 0)
   {
      TRACE_ERROR("Format FS");
      return -1;
   }

   return 0;
}

static uint8_t pagebuf[S25FL127_PAGE_SIZE];

static s32_t spiffs_drv_read(u32_t addr, u32_t size, u8_t *dst)
{
   return (df_read(addr, dst, size) == size) ? 0 : -1;   
}

static s32_t spiffs_drv_write(u32_t addr, u32_t size, u8_t *src)
{
   return (df_write(addr, src, size) == size ? 0 : -1);
}

static s32_t spiffs_drv_erase(u32_t addr, u32_t size)
{
   return df_erase_block(addr / SPIFFS_BLOCK_SIZE);
}


#if TEST_SPIFFS
int spiffs_test(int fileindex, int filesize)
{
   
   int i;
   spiffs_file fd;
   int count, len;
   uint32_t start_tm, total, used;
   static int ncycle = 0;
   char fname[32];
   uint8_t buf[SPIFFS_PAGE_SIZE];

   ee_snprintf(fname, sizeof(fname), "test%d", fileindex);

   TRACE("Opening file %s ...", fname);

   //
   // Write data
   //
   fd = SPIFFS_open(&fs, fname, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
   if (fd < 0)
   {
      TRACE_ERROR("Create file '%s'", fname);
      return -1;
   }
   
   while(1)
   {
      start_tm = hal_time_ms();
      SPIFFS_info(&fs, &total, &used);

      TRACE("Test[%d]  file: '%s' size: %d  ", ncycle++, fname, filesize);
      TRACE("Free blocks:                 %u", fs.free_blocks);
      TRACE("Total space size:            %u", total);
      TRACE("Used space size:             %u", used);
      TRACE("Free space size:             %u", total - used);

      srand(start_tm);
      count = filesize;
      while(count > 0)
      {
         len = count > sizeof(buf) ? sizeof(buf) : count;
         for (i = 0; i < len; i++)
         {
            buf[i] = rand() % 256;
         }

         if (SPIFFS_write(&fs, fd, buf, len) < 0)
         {
            TRACE_ERROR("Write errno %d", SPIFFS_errno(&fs));
            SPIFFS_close(&fs, fd);
            return -1;
         }

         TRACE_PRINTF("\r");
         TRACE_PRINTFF("Write %d/%d ... ", len, count);

         count -= len;
      }
      SPIFFS_close(&fs, fd);
      TRACE_PRINTF("done\r\n");

      //
      // Read data
      //
      srand(start_tm);
      count = 0;

      fd = SPIFFS_open(&fs, fname, SPIFFS_RDWR, 0);
      if (fd < 0)
      {
         TRACE_ERROR("Open file");
         return -1;
      }

      while((len = SPIFFS_read(&fs, fd, buf, sizeof(buf))) > 0)
      {
         for (i = 0; i < len; i++)
         {
            if (buf[i] != rand() % 256)
            {
               TRACE_ERROR("Compare byte offset: %d", i);
               SPIFFS_close(&fs, fd);
               return -1;
            }
         }

         count += len;

         TRACE_PRINTF("\r");
         TRACE_PRINTFF("Read %d/%d bytes ... ", len, count);
      }
      TRACE_PRINTF("done\r\n");

      if (len < 0)
      {
         if (SPIFFS_errno(&fs) != SPIFFS_ERR_END_OF_OBJECT )
         {
            TRACE_ERROR("Read errno %d", SPIFFS_errno(&fs));
            return -1;
         }
      }

      TRACE("Test done: %d ms\r\n", hal_time_ms() - start_tm);

      // Seek to begin of file
      if (SPIFFS_lseek(&fs, fd, 0, SPIFFS_SEEK_SET) < 0)
      {
         TRACE_ERROR("Seek to begin");
         return -1;
      }
   }

   SPIFFS_close(&fs, fd);

   return 0;
}
#endif  // TEST_SPIFFS