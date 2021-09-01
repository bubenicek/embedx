/**
 * \file nand_hynix.c    \brief NAND flash HYNIX model HY27UF08-1G2A
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "debug.h"

#include "uffs_config.h"
#include "uffs/uffs_os.h"
#include "uffs/uffs_device.h"
#include "uffs/uffs_flash.h"
#include "uffs/uffs_mtb.h"
#include "uffs/uffs_fs.h"
#include "uffs/uffs_fd.h"

#include "fsmc_nand.h"

#define PFX "ndrv: "
#define TRACE_TAG "nand_drv"

//
// Change these parameters to fit your nand flash specification
//
#define TOTAL_BLOCKS          1024     // 32MB
#define PAGE_DATA_SIZE        2048
#define PAGE_SPARE_SIZE       64
#define PAGES_PER_BLOCK       16      // we are using 16 pages per block instead physical 64 pages because we have performance problem !!
#define PAGE_SIZE		         (PAGE_DATA_SIZE + PAGE_SPARE_SIZE)
#define BLOCK_DATA_SIZE       (PAGE_DATA_SIZE * PAGES_PER_BLOCK)
#define BAD_BLOCK_OFFSET      0

#define NR_PARTITION	         1								      /* total partitions */
#define PAR_1_BLOCKS	         TOTAL_BLOCKS					   /* partition 1 */
#define PAR_2_BLOCKS	         (TOTAL_BLOCKS - PAR_1_BLOCKS)	/* partition 2 */

static struct uffs_StorageAttrSt g_my_flash_storage = {0};

/* define mount table */
static uffs_Device device_1 = {0};

static uffs_MountTable mount_table[] =
{
   { &device_1,  0, PAR_1_BLOCKS - 1, "/"},
   { NULL, 0, 0, NULL }
};

/** Static alloc the memory for each partition */
static __attribute__((section(".ccmram"))) int static_buffer_par1[UFFS_STATIC_BUFF_SIZE(PAGES_PER_BLOCK, PAGE_SIZE, PAR_1_BLOCKS) / sizeof(int)];

/** working page buffer */
static uint8_t pagebuf[PAGE_DATA_SIZE];

//-----------------------------------------------------------------------------

static int nand_read_page(uffs_Device *dev, u32 block, u32 page, u8 *data, int data_len, u8 *ecc, u8 *spare, int spare_len)
{
   NAND_ADDRESS addr;
   u8 *pdata;
   int ret = UFFS_FLASH_NO_ERR;

   // Setup address
   addr.Zone = 0;
   addr.Block = block;
   addr.Page = page;

   if (data && data_len > 0)
   {
      if (data_len < PAGE_DATA_SIZE)
      {
         pdata = pagebuf;
      }
      else
      {
         pdata = data;
      }

      // Read data Page Content
      if (FSMC_NAND_ReadSmallPage(pdata, addr, 1) != NAND_READY)
      {
         ret = UFFS_FLASH_IO_ERR;
      }

      if (ret == UFFS_FLASH_NO_ERR && pdata != data)
      {
         memcpy(data, pagebuf, data_len);
      }
   }

   if (spare && spare_len > 0)
   {
      pdata = &pagebuf[BAD_BLOCK_OFFSET+1];

      // Read spare
      if (FSMC_NAND_ReadSpareArea(pdata, addr, 1) == NAND_READY)
      {
         memcpy(spare, pdata, spare_len);
      }
      else
      {
         ret = UFFS_FLASH_IO_ERR;
      }
   }

   if (data == NULL && spare == NULL)
   {
      // Check block status: bad or good
      memset(pagebuf, 0, UFFS_MAX_SPARE_SIZE);

      // Read spare
      if (FSMC_NAND_ReadSpareArea(pagebuf, addr, 1) == NAND_READY)
      {
         //ret = pagebuf[dev->attr->block_status_offs] == 0xFF ? UFFS_FLASH_NO_ERR : UFFS_FLASH_BAD_BLK;
         ret = UFFS_FLASH_NO_ERR;
      }
      else
      {
         TRACE_ERROR("Read spare block: %d", block);
         ret = UFFS_FLASH_IO_ERR;
      }
   }

   TRACE("%s: ret=%d  block=%d  page=%d  data=%p  datalen=%d  ecc=%p  spare=%p  spare_len=%d",
      __FUNCTION__, ret, block, page, data, data_len, ecc, spare, spare_len);

   return ret;
}

static int nand_write_page(uffs_Device *dev, u32 block, u32 page, const u8 *data, int data_len, const u8 *spare, int spare_len)
{
   NAND_ADDRESS addr;
   u8 *pdata;
   int ret = UFFS_FLASH_NO_ERR;

   // Setup address
   addr.Zone = 0;
   addr.Block = block;
   addr.Page = page;

   if (data && data_len > 0)
   {
      if (data_len < PAGE_DATA_SIZE)
      {
         memcpy(pagebuf, data, data_len);
         pdata = pagebuf;
      }
      else
      {
         pdata = (u8 *)data;
      }

      // Write data page (DATA ONLY)
      if (FSMC_NAND_WriteSmallPage(pdata, addr, 1) != NAND_READY)
      {
         ret = UFFS_FLASH_IO_ERR;
      }
   }

   if (spare && spare_len > 0)
   {
      pdata = &pagebuf[BAD_BLOCK_OFFSET+1];
      memcpy(pdata, spare, spare_len);

      // Write spare page (SPARE ONLY + offset to protect Bad Block Info)
      if (FSMC_NAND_WriteSpareArea(pdata, addr, 1) != NAND_READY)
      {
         ret = UFFS_FLASH_IO_ERR;
      }
   }

   if (data == NULL && spare == NULL)
   {
      // Mark bad block

      memset(pagebuf, 0xFF, UFFS_MAX_SPARE_SIZE);
      pagebuf[dev->attr->block_status_offs] =  0x00;

      // Write spare page (SPARE ONLY + offset to protect Bad Block Info)
      if (FSMC_NAND_WriteSpareArea(pagebuf, addr, 1) != NAND_READY)
      {
         ret = UFFS_FLASH_IO_ERR;
      }

      TRACE("Mark BAD block %d !", block);
   }

   TRACE("%s: ret=%d  block=%d  page=%d  data=%p  datalen=%d  spare=%p  spare_len=%d",
       __FUNCTION__, ret, block, page, data, data_len, spare, spare_len);

   return ret;
}

static int nand_erase_block(uffs_Device *dev, u32 block)
{
   NAND_ADDRESS addr;

   TRACE("Erase block: %d", block);

   // Setup Address Block
   addr.Zone = 0;
   addr.Block = block;
   addr.Page = 0;

   // FLASH NAND Device Erase Block Content's
   if (FSMC_NAND_EraseBlock(addr) != NAND_READY)
   {
      return UFFS_FLASH_IO_ERR;
   }

   return UFFS_FLASH_NO_ERR;
}

static int nand_init_flash(uffs_Device *dev)
{
   // Initialise NAND Driver Interface
   FSMC_NAND_Init();

   TRACE("NAND flash initialized");

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
   attr->ecc_opt = UFFS_ECC_SOFT;              /* ecc option */
   attr->layout_opt = UFFS_LAYOUT_UFFS;        /* let UFFS do the spare layout */
}

static URET nand_InitDevice(uffs_Device *dev)
{
   dev->attr = &g_my_flash_storage;			// NAND flash attributes
   dev->attr->_private = NULL;                 // hook nand_chip data structure to attr->_private
   dev->ops = &g_my_nand_ops;					// NAND driver

   return U_SUCC;
}

static URET nand_ReleaseDevice(uffs_Device *dev)
{
   return U_SUCC;
}

static void nand_WdgReset(uffs_Device *dev)
{
   // TODO: watchdog reset
}

int uffs_init_filesystem(void)
{
   uffs_MountTable *mtbl = &(mount_table[0]);

   // setup debug output as early as possible
   uffs_SetupDebugOutput();

   // setup nand storage attributes
   setup_flash_storage(&g_my_flash_storage);

   // setup memory allocator
   uffs_MemSetupStaticAllocator(&device_1.mem, static_buffer_par1, sizeof(static_buffer_par1));

   // register mount table
   while(mtbl->dev)
   {
      // setup device init/release entry
      mtbl->dev->Init = nand_InitDevice;
      mtbl->dev->Release = nand_ReleaseDevice;
      mtbl->dev->WdgReset = nand_WdgReset;
      uffs_RegisterMountTable(mtbl);
      mtbl++;
   }

   // mount partitions
   for (mtbl = &(mount_table[0]); mtbl->mount != NULL; mtbl++)
   {
      uffs_Mount(mtbl->mount);
   }

   return uffs_InitFileSystemObjects() == U_SUCC ? 0 : -1;
}

int uffs_release_filesystem(void)
{
   uffs_MountTable *mtb;
   int ret = 0;

   // unmount parttions
   for (mtb = &(mount_table[0]); ret == 0 && mtb->mount != NULL; mtb++)
   {
      ret = uffs_UnMount(mtb->mount);
   }

   // release objects
   if (ret == 0)
      ret = (uffs_ReleaseFileSystemObjects() == U_SUCC ? 0 : -1);

   return ret;
}

int uffs_erase_filesystem(void)
{
   int ix;
   uint8_t nand_init = (NULL != mount_table[0].dev->Init);

   if (!nand_init)
   {
      setup_flash_storage(&g_my_flash_storage);

      if (0 != nand_InitDevice(&device_1))
         return -1;

      if (0 != nand_init_flash(&device_1))
         return -1;
   }

   for (ix = 0; ix < TOTAL_BLOCKS; ix++)
   {
      // Check if block is not BAD
      if (UFFS_FLASH_NO_ERR == nand_read_page(&device_1, ix, 0, NULL, 0, NULL, NULL, 0))
      {
         nand_erase_block(&device_1, ix);
      }
      else
      {
         TRACE("Block %d is BAD !", ix);
      }
   }

   if (!nand_init)
   {
      if (0 != nand_release_flash(&device_1))
         return -1;
   }

   return 0;
}
