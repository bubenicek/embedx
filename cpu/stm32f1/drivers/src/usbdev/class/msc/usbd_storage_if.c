/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @brief          : Memory management layer
  ******************************************************************************
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "system.h"

#if defined(CFG_USB_STORAGE) && (CFG_USB_STORAGE == 1)

#include "usbd_storage_if.h"
#include "diskio.h"

#define TRACE_TAG "usbstorage"

#define STORAGE_LUN_NBR                  1
#define STORAGE_BLK_NBR                  (CFG_FS_STORAGE_SIZE / STORAGE_BLK_SIZ)
#define STORAGE_BLK_SIZ                  512

/* Handle for USB Full Speed IP */
USBD_HandleTypeDef  *hUsbDevice_0;

/* USB Mass storage Standard Inquiry Data */
const int8_t  STORAGE_Inquirydata_FS[] =  //36
{
   /* LUN 0 */
   0x00,
   0x80,
   0x02,
   0x02,
   (STANDARD_INQUIRY_DATA_LEN - 5),
   0x00,
   0x00,
   0x00,
   'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
   'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
   '0', '.', '0' ,'1',                     /* Version      : 4 Bytes */
};

/**
  * @}
  */

/** @defgroup USBD_STORAGE_IF_Exported_Variables
  * @{
  */
extern USBD_HandleTypeDef hUsbDeviceFS;

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_FunctionPrototypes
  * @{
  */
static int8_t STORAGE_Init_FS (uint8_t lun);
static int8_t STORAGE_GetCapacity_FS (uint8_t lun,
                                      uint32_t *block_num,
                                      uint16_t *block_size);
static int8_t  STORAGE_IsReady_FS (uint8_t lun);
static int8_t  STORAGE_IsWriteProtected_FS (uint8_t lun);
static int8_t STORAGE_Read_FS (uint8_t lun,
                               uint8_t *buf,
                               uint32_t blk_addr,
                               uint16_t blk_len);
static int8_t STORAGE_Write_FS (uint8_t lun,
                                uint8_t *buf,
                                uint32_t blk_addr,
                                uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS (void);


USBD_StorageTypeDef USBD_Storage_Interface_fops_FS =
{
   STORAGE_Init_FS,
   STORAGE_GetCapacity_FS,
   STORAGE_IsReady_FS,
   STORAGE_IsWriteProtected_FS,
   STORAGE_Read_FS,
   STORAGE_Write_FS,
   STORAGE_GetMaxLun_FS,
   (int8_t *)STORAGE_Inquirydata_FS,
};


inline static void fs_blink_led(void)
{
   static uint8_t cnt = 0;
   if (++cnt == 4)
   {
      hal_led_toggle(LED_DATA);
      cnt = 0;
   }
}

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : STORAGE_Init_FS
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_Init_FS (uint8_t lun)
{
   TRACE("Init FS");

   if (disk_status(0) == STA_NOINIT)
      disk_initialize(0);

   return disk_status(0) == RES_OK ? USBD_OK : USBD_FAIL; 
}

/*******************************************************************************
* Function Name  : STORAGE_GetCapacity_FS
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_GetCapacity_FS (uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
   *block_num  = STORAGE_BLK_NBR;
   *block_size = STORAGE_BLK_SIZ;
   return USBD_OK;
}

/*******************************************************************************
* Function Name  : STORAGE_IsReady_FS
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t  STORAGE_IsReady_FS (uint8_t lun)
{
   hal_led_set(LED_DATA, 0);
   return disk_status(0) == RES_OK ? USBD_OK : USBD_FAIL; 
}

/*******************************************************************************
* Function Name  : STORAGE_IsWriteProtected_FS
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t  STORAGE_IsWriteProtected_FS (uint8_t lun)
{
   return 0;
}

/*******************************************************************************
* Function Name  : STORAGE_Read_FS
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_Read_FS (uint8_t lun,
                        uint8_t *buf,
                        uint32_t blk_addr,
                        uint16_t blk_len)
{    
   fs_blink_led();
   return (disk_read(0, buf, blk_addr, blk_len) == RES_OK) ? USBD_OK : USBD_FAIL;
}

/*******************************************************************************
* Function Name  : STORAGE_Write_FS
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_Write_FS (uint8_t lun,
                         uint8_t *buf,
                         uint32_t blk_addr,
                         uint16_t blk_len)
{
   fs_blink_led();
   return (disk_write(0, buf, blk_addr, blk_len) == RES_OK) ? USBD_OK : USBD_FAIL;
}

/*******************************************************************************
* Function Name  : STORAGE_GetMaxLun_FS
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_GetMaxLun_FS (void)
{
   return (STORAGE_LUN_NBR - 1);
}

#endif  // CFG_USB_STORAGE