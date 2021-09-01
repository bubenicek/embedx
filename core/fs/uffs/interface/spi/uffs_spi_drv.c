
#include "system.h"
#include "dev/mem/S25FL127.h"

#define UFFS_DISABLE_TYPEDEF     1
#include "uffs_config.h"
#include "uffs/uffs_os.h"
#include "uffs/uffs_device.h"
#include "uffs/uffs_flash.h"
#include "uffs/uffs_mtb.h"
#include "uffs/uffs_fs.h"
#include "uffs/uffs_fd.h"

#ifndef TRACE_TAG
#define TRACE_TAG    "uffs_spi"
#endif // TRACE_TAG

/*
#ifndef CONFIG_ENABLE_UFFS_DEBUG_MSG
#undef TRACE
#define TRACE(...)
#endif // CONFIG_ENABLE_UFFS_DEBUG_MSG
*/

#define CFG_UFFS_PRINT_STATINFO   1
#define CFG_UFFS_TEST             0

//
// SPI dataflsh interface
//
#define df_init()                   s25fl127_init()
#define df_read_page(_addr, _buf)   s25fl127_read_page(_addr, _buf)
#define df_write_page(_addr, _buf)  s25fl127_write_page(_addr, _buf)
#define df_erase_block(_block)      s25fl127_erase_sector(_block)

//
// Change these parameters to fit your nand flash specification
//
#define TOTAL_BLOCKS          S25FL127_NUM_SECTORS
#define PAGE_SIZE		         (S25FL127_PAGE_SIZE * 8)      // 2048 bytes virtual page size
#define BLOCK_SIZE            S25FL127_SECTOR_SIZE
#define PHY_PAGES_PER_PAGE    (PAGE_SIZE / S25FL127_PAGE_SIZE) // Number of physical pages per virtual page

#define PAGES_PER_BLOCK       (BLOCK_SIZE / PAGE_SIZE)
#define PAGE_SPARE_SIZE       10
#define PAGE_DATA_SIZE        (PAGE_SIZE - PAGE_SPARE_SIZE)
#define BAD_BLOCK_OFFSET      0

#define NR_PARTITION	         1								      // total partitions
#define PAR_1_BLOCKS	         TOTAL_BLOCKS					   // partition 1 


static CFG_UFFS_RAM_SECTION struct uffs_StorageAttrSt flash_storage = {0};

static CFG_UFFS_RAM_SECTION uffs_Device device_1 = {0};

/** Static alloc the memory for each partition */
static CFG_UFFS_RAM_SECTION int static_buffer_par1[UFFS_STATIC_BUFF_SIZE(PAGES_PER_BLOCK, PAGE_SIZE, PAR_1_BLOCKS) / sizeof(int)];

/** working page buffer */
static CFG_UFFS_RAM_SECTION uint8_t pagebuf[PAGE_SIZE];

/** Filesystem start block = 1, service page block = 0 */
static uffs_MountTable mount_table[] =
{
   { &device_1,  1, PAR_1_BLOCKS - 1, "/"},
   { NULL, 0, 0, NULL }
};

//-----------------------------------------------------------------------------

static int nand_read_page(uffs_Device *dev, u32 block, u32 page, u8 *data, int data_len, u8 *ecc, u8 *spare, int spare_len)
{
   int i;
   uint8_t *pbuf;
   uint32_t phy_page = ((block * PAGES_PER_BLOCK) + page) * (PAGE_SIZE / S25FL127_PAGE_SIZE);

   if (data == NULL && spare == NULL)
   {
      // Check block status: bad or good, block is always good
      return 0;
   }

   if (data && data_len > 0)
   {
      ASSERT(data_len <= PAGE_DATA_SIZE);

      // Read phy pages
      for (i = 0, pbuf = pagebuf; i < PAGE_SIZE / S25FL127_PAGE_SIZE; i++, pbuf += S25FL127_PAGE_SIZE)
      {
         df_read_page(phy_page+i, pbuf);
      }

      memcpy(data, pagebuf, data_len);
   }

   if (spare && spare_len > 0)
   {
      // Read spare
      ASSERT(spare_len <= PAGE_SPARE_SIZE);

      if (data == NULL)
      {
         // Read only last page with spare data
         df_read_page(phy_page + PHY_PAGES_PER_PAGE - 1, &pagebuf[S25FL127_PAGE_SIZE * (PHY_PAGES_PER_PAGE - 1)]);
      }

      memcpy(spare, &pagebuf[PAGE_DATA_SIZE], spare_len);
   }

   //TRACE("Read phy_page[%d] : block=%d  page=%d  data=%p  datalen=%d  ecc=%p  spare=%p  spare_len=%d", phy_page, block, page, data, data_len, ecc, spare, spare_len);

   return 0;
}

static int nand_write_page(uffs_Device *dev, u32 block, u32 page, const u8 *data, int data_len, const u8 *spare, int spare_len)
{
   int i;
   uint8_t *pbuf;
   uint32_t phy_page = ((block * PAGES_PER_BLOCK) + page) * (PAGE_SIZE / S25FL127_PAGE_SIZE);

   if (data == NULL && spare == NULL)
   {
      // Mark bad block, nothing todo
      TRACE("!!! Mark BAD block %d not implemented !!!", block);
      return 0;
   }

   if (data && data_len > 0)
   {
      // Store data to page
      ASSERT(data_len <= PAGE_DATA_SIZE);
      memcpy(pagebuf, data, data_len);
   }

   if (spare && spare_len > 0)
   {
      // Append new spare
      ASSERT(spare_len <= PAGE_SPARE_SIZE);
      memcpy(&pagebuf[PAGE_DATA_SIZE], spare, spare_len);
   }

   // Write phy pages
   for (i = 0, pbuf = pagebuf; i < PAGE_SIZE / S25FL127_PAGE_SIZE; i++, pbuf += S25FL127_PAGE_SIZE)
   {
      df_write_page(phy_page+i, pbuf);
   }

   //TRACE("Write phy_page[%d] :  block=%d  page=%d  data=%p  datalen=%d  spare=%p  spare_len=%d", phy_page, block, page, data, data_len, spare, spare_len);

   return 0;
}

static int nand_erase_block(uffs_Device *dev, u32 block)
{
   //TRACE("Erase block: %d", block);

   if (df_erase_block(block) != 0)
   {
      return -1;
   }

   return 0;
}

static int nand_init_flash(uffs_Device *dev)
{
   // Initialise NAND Driver Interface
   if (df_init() != 0)
   {
      return -1;
   }

   TRACE("SPI flash init");

   return 0;
}

// release your hardware here
static int nand_release_flash(uffs_Device *dev)
{
   return 0;
}

static uffs_FlashOps g_my_nand_ops =
{
   nand_init_flash,	// InitFlash()
   nand_release_flash,	// ReleaseFlash()
   nand_read_page,		// ReadPage()
   NULL,				// ReadPageWithLayout
   nand_write_page,	// WritePage()
   NULL,				// WirtePageWithLayout
   NULL,				// IsBadBlock(), let UFFS take care of it.
   NULL,				// MarkBadBlock(), let UFFS take care of it.
   nand_erase_block,	// EraseBlock()
};

/////////////////////////////////////////////////////////////////////////////////


static void setup_flash_storage(struct uffs_StorageAttrSt *attr)
{
   memset(attr, 0, sizeof(struct uffs_StorageAttrSt));

   // setup NAND flash attributes.
   attr->total_blocks = TOTAL_BLOCKS;			/* total blocks */
   attr->page_data_size = PAGE_DATA_SIZE;		/* page data size */
   attr->pages_per_block = PAGES_PER_BLOCK;	/* pages per block */
   attr->spare_size = PAGE_SPARE_SIZE;		  	/* page spare size */
   attr->block_status_offs = BAD_BLOCK_OFFSET; /* block status offset in spare */
   attr->ecc_opt = UFFS_ECC_NONE;              /* ecc option */
   attr->layout_opt = UFFS_LAYOUT_UFFS;        /* let UFFS do the spare layout */
}

static URET nand_InitDevice(uffs_Device *dev)
{
   dev->attr = &flash_storage;			// NAND flash attributes
   dev->attr->_private = NULL;                 // hook nand_chip data structure to attr->_private
   dev->ops = &g_my_nand_ops;					// NAND driver

   return U_SUCC;
}

static URET nand_ReleaseDevice(uffs_Device *dev)
{
   return U_SUCC;
}

int uffs_init_filesystem(void)
{
   uffs_MountTable *mtbl = &(mount_table[0]);
   
   memset(&device_1, 0, sizeof(device_1));
   memset(&flash_storage, 0, sizeof(flash_storage));

   // setup debug output as early as possible
   uffs_SetupDebugOutput();

   // setup nand storage attributes
   setup_flash_storage(&flash_storage);

   // setup memory allocator
   uffs_MemSetupStaticAllocator(&device_1.mem, static_buffer_par1, sizeof(static_buffer_par1));

   // register mount table
   while(mtbl->dev)
   {
      // setup device init/release entry
      mtbl->dev->Init = nand_InitDevice;
      mtbl->dev->Release = nand_ReleaseDevice;
      uffs_RegisterMountTable(mtbl);
      mtbl++;
   }

   // mount partitions
   for (mtbl = &(mount_table[0]); mtbl->mount != NULL; mtbl++)
   {
      uffs_Mount(mtbl->mount);
   }

   if (uffs_InitFileSystemObjects() == U_SUCC)
   {
#if CFG_UFFS_PRINT_STATINFO
      TRACE("UFFS total space: %d", uffs_space_total("/"));
      TRACE("UFFS used space: %d", uffs_space_used("/"));
      TRACE("UFFS free space: %d", uffs_space_free("/"));

      // List root files and dirs
      uffs_DIR *dir;
      struct uffs_dirent *dirent;
      struct uffs_stat stbuf;
      char path[128];

      dir = uffs_opendir("/");
      if (dir == NULL)
      {
         TRACE_ERROR("Open root dir");
         return -1;
      }

      TRACE("Filename                     fsize   nblocks ");
      TRACE("---------------------------------------------");
      while ((dirent = uffs_readdir(dir)) != NULL)
      {
         snprintf(path, sizeof(path), "/%s", dirent->d_name);
         if (dirent->d_type == 1)
         {
            VERIFY(uffs_stat(path, &stbuf) == 0);
            TRACE("%-30s %-8d %-8d", path, stbuf.st_size, stbuf.st_blocks, dirent->d_type);
         }
         else
         {
            TRACE("%-30s [DIR]", path);
         }
      }
      uffs_closedir(dir);
#endif

#if CFG_UFFS_TEST
      int fileindex = 0;
      while(1)
      {
         ASSERT(uffs_test(fileindex, rand() % (256 * 1024)) == 0);
         fileindex++;
      }

#endif // UFFS_TEST

      return 0;
   }
   else
   {
      return -1;
   }
}

int uffs_deinit_filesystem(void)
{
   uffs_MountTable *mtb;
   int ret = 0;

   // unmount parttions
   for (mtb = &(mount_table[0]); ret == 0 && mtb->mount != NULL; mtb++)
   {
      uffs_flush_all(mtb->mount);
      ret = uffs_UnMount(mtb->mount);
      TRACE("Unmount ret: %d", ret);
   }

   // release objects
   if (ret == 0)
      ret = (uffs_ReleaseFileSystemObjects() == U_SUCC ? 0 : -1);

   TRACE("Filesystem deinit");

   return ret;
}

int uffs_erase_filesystem(void)
{
   int ix;

   uint8_t nand_init = (NULL != mount_table[0].dev->Init);

   if (!nand_init)
   {
      setup_flash_storage(&flash_storage);

      if (0 != nand_InitDevice(&device_1))
         return -1;

      if (0 != nand_init_flash(&device_1))
         return -1;
   }

   for (ix = mount_table[0].start_block; ix <= mount_table[0].end_block; ix++)
   {
      TRACE_PRINTF("\r");
      TRACE_PRINTFF("Erasing block %d ... ", ix);
      nand_erase_block(&device_1, ix);
      hal_wdg_reset();
   }

   TRACE_PRINTF("done\r\n");

   if (!nand_init)
   {
      if (0 != nand_release_flash(&device_1))
         return -1;
   }

   return 0;
}

#if CFG_UFFS_TEST == 1
int uffs_test(int fileindex, int filesize)
{
   static uint8_t buf[1024];
   int i;
   int fd;
   int count, len;
   static char fname[32];
   uint32_t start_tm;
   static uint8_t ncycle = 0;

   start_tm = hal_time_ms();
   sprintf(fname, "/test%d", fileindex);

   TRACE("Test[%d] file '%s' size: %d is running ...", ++ncycle, fname, filesize);
   TRACE("UFFS total space: %d", uffs_space_total("/"));
   TRACE("UFFS used space: %d", uffs_space_used("/"));
   TRACE("UFFS free space: %d", uffs_space_free("/"));

   //
   // Write data
   //
   fd = uffs_open(fname, UO_CREATE | UO_TRUNC | UO_RDWR, 0);
   if (fd < 0)
   {
      TRACE_ERROR("Create file '%s'  err: %d", fname, fd);
      return -1;
   }

   srand(start_tm);
   count = filesize;
   while(count > 0)
   {
      len = count > sizeof(buf) ? sizeof(buf) : count;
      for (i = 0; i < len; i++)
      {
         buf[i] = rand() % 256;
      }

      if (uffs_write(fd, buf, len) < 0)
      {
         TRACE_ERROR("Write");
         uffs_close(fd);
         return -1;
      }

      TRACE_PRINTF("\r");
      TRACE_PRINTFF("Write %d/%d ...", len, count);

      count -= len;
   }
   TRACE_PRINTF(" done\r\n");
   uffs_close(fd);


   //
   // Read data
   //
   srand(start_tm);
   count = 0;

   fd = uffs_open(fname, UO_RDWR, 0);
   if (fd < 0)
   {
      TRACE_ERROR("Open file");
      return -1;
   }

   while((len = uffs_read(fd, buf, sizeof(buf))) > 0)
   {
      for (i = 0; i < len; i++)
      {
         if (buf[i] != rand() % 256)
         {
            TRACE_ERROR("Compare byte offset: %d  %d != %d", i, buf[i], ncycle);
            uffs_close(fd);
            return -1;
         }
      }

      TRACE_PRINTF("\r");
      TRACE_PRINTFF("Read %d/%d ...", len, count);

      count += len;
   }
   TRACE_PRINTF(" done\r\n");

   uffs_close(fd);

   if (len < 0)
   {
      TRACE_ERROR("Read");
      return -1;
   }

   TRACE("Test done: %d ms\r\n", hal_time_ms() - start_tm);
   
   uffs_remove(fname);

   return 0;
}
#endif
