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
#include "diskio.h"		/* FatFs lower layer API */
#include "S25FL127.h"

#define TRACE_TAG    "diskio"

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
   return s25fl127_init() == 0 ? RES_OK : RES_ERROR;
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
   TRACE("%s: sector: %d  count: %d", __FUNCTION__, sector, count);
        
   if (s25fl127_read(sector * _MAX_SS, buf, count * _MAX_SS) != 0)
   {
      TRACE_ERROR("Diskio read sector: %d  count: %d", sector, count);
      return RES_ERROR;
   }
   
	return RES_OK;
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
   TRACE("%s: sector: %d  count: %d", __FUNCTION__, sector, count);

   if (s25fl127_write(sector * _MAX_SS, (uint8_t *)buf, count * _MAX_SS) != 0)
   {
      TRACE_ERROR("Diskio write sector: %d  count: %d", sector, count);
      return RES_ERROR;
   }
   
	return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
   TRACE("%s: cmd: %d", __FUNCTION__, cmd);
   
	return RES_PARERR;
}
