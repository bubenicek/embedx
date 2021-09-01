/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "system.h"

#include "ff.h"
#include "diskio.h"		
#include "spiffs.h"
#include "spiffs_driver.h"

#define TRACE_TAG    "diskio"

static spiffs *spifs = NULL;
static spiffs_file fd;  
   
/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{  
   if (spifs != NULL)
      return RES_OK;
        
   TRACE("%s: ", __FUNCTION__);
   
   // Mount FS
   if (spiffs_mount(&spifs) != 0)
      throw_exception(fail_mount);

   // Open FAT fs file
   if ((fd = SPIFFS_open(spifs, CFG_FS_DISK_FILENAME, SPIFFS_RDWR, 0)) < 0)
   {
      TRACE_ERROR("Can't open fatfs file");
      throw_exception(fail_open);
   }
         
   return RES_OK;

fail_open:
   VERIFY(spiffs_unmount(spifs) == 0);   
fail_mount:
   spifs = NULL;
   hal_led_blink(LED_ERROR, 1, 50, 100);
   return STA_NOINIT;      
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return spifs == NULL ? STA_NOINIT : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buf,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
   int len = count * _MAX_SS;
   int pos = sector * _MAX_SS;
  
   if (SPIFFS_tell(spifs, fd) != pos)
   {
      if (SPIFFS_lseek(spifs, fd, pos, SPIFFS_SEEK_SET) < 0)
      {
         TRACE_ERROR("Seek to file offset");
         throw_exception(fail);
      }
   }
          
   if (SPIFFS_read(spifs, fd, buf, len) != len)
   {
      TRACE_ERROR("Diskio read sector: %d  count: %d  errno: %d", sector, count, SPIFFS_errno(spifs));
      throw_exception(fail);
   }
   
	return RES_OK;

fail:
   hal_led_blink(LED_ERROR, 1, 50, 100);
   return RES_ERROR;	
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buf,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
   int len = count * _MAX_SS;
   int pos = sector * _MAX_SS;
  
   if (SPIFFS_tell(spifs, fd) != pos)
   {
      if (SPIFFS_lseek(spifs, fd, pos, SPIFFS_SEEK_SET) < 0)
      {
         TRACE_ERROR("Seek to file offset, errno: %d", SPIFFS_errno(spifs));
         throw_exception(fail);
      }
   }
   
   if (SPIFFS_write(spifs, fd, (uint8_t *)buf, len) != len)
   {
      TRACE_ERROR("Diskio write sector: %d  count: %d  errno: %d", sector, count, SPIFFS_errno(spifs));
      throw_exception(fail);
   }
   
	return RES_OK;

fail:
   hal_led_blink(LED_ERROR, 1, 50, 100);
   return RES_ERROR;	
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buf		/* Buffer to send/receive control data */
)
{
   //TRACE("%s: cmd: %d", __FUNCTION__, cmd);
   
   switch(cmd)
   {
      case CTRL_SYNC:   
      {
         if (SPIFFS_fflush(spifs, fd) != 0)
         {
            TRACE_ERROR("CTRL_SYNC");
            return RES_ERROR;
         }
      }
      break;
      
      case GET_SECTOR_SIZE:
      {
         DWORD *pv = buf;
         *pv = 512;
      }
      break;

      case GET_SECTOR_COUNT:
      {
         DWORD *pv = buf;
         *pv = 8192;
      }
      break;

      case GET_BLOCK_SIZE:
      {
         DWORD *pv = buf;
         *pv = 512;
      }
      break;
      
      case CTRL_FORMAT:
      {
         int size;
         uint32_t start_tm;
         uint8_t buf[1024];

         if (spifs == NULL)
         {
            if (spiffs_mount(&spifs) != 0)
               throw_exception(fail_mount);
         }

         TRACE_PRINTFF("Low level formating ... ");
         VERIFY(spiffs_unmount(spifs) == 0);
         VERIFY(SPIFFS_format(spifs) == 0);
         TRACE_PRINTF("done\n");

         // Mount FS
         if (spiffs_mount(&spifs) != 0)
            throw_exception(fail_mount);

         TRACE("Init disk file ...");
         start_tm = hal_time_ms();
      
         // Create new file
         if ((fd = SPIFFS_open(spifs, CFG_FS_DISK_FILENAME, SPIFFS_CREAT | SPIFFS_RDWR, 0)) < 0)
         {
            TRACE_ERROR("Create fatfs disk file '%s'", CFG_FS_DISK_FILENAME);
            throw_exception(fail_open);
         }
      
         memset(buf, 0, sizeof(buf));
      
         // Zero file
         for (size = CFG_FS_STORAGE_SIZE; size > 0; size -= sizeof(buf))
         {
            if (SPIFFS_write(spifs, fd, buf, sizeof(buf)) != sizeof(buf))
            {
               TRACE_ERROR("Diskio write");
               throw_exception(fail_init);
            }
         }
      
         TRACE("Init done, tmlen: %d ms", hal_time_ms() - start_tm);
         return RES_OK;
         
fail_init:
         VERIFY(SPIFFS_close(spifs, fd));
fail_open:
         VERIFY(spiffs_unmount(spifs) == 0);   
fail_mount:   
         return RES_NOTRDY;
      }
      break;

      default:
         TRACE_ERROR("Not supported cmd: %d", cmd);
         return RES_PARERR;
   }
   
	return RES_OK;
}
