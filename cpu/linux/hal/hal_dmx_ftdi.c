
#include "system.h"
#include "ftdi.h"

#define TRACE_TAG "hal-dmx"
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#define FTDI_VID                        0x0403  // FTDI Vendor ID
#define FTDI_PID                        0x6001  // FTDI Product ID

static struct ftdi_context ftHandle;


/** Initialize DMX device */
int hal_dmx_init(hal_dmx_t dmx, uint32_t dmx_break_delay)
{
   if (ftdi_init(&ftHandle) < 0)
   {
      TRACE_ERROR("ftdi init error");
      return -1;
   }
   
   // Open FTDI device
   if (ftdi_usb_open(&ftHandle, FTDI_VID, FTDI_PID) < 0)
   {
      TRACE_ERROR("Open ftdi error");
      return -1;   
   }

   // Reset the devices
   if (ftdi_usb_reset(&ftHandle) < 0)
   {
      TRACE_ERROR("FT_ResetDevice error - Device not responding");
      return -1;
   }

   // Set the baud rate 12 will give us 250Kbits
   if (ftdi_set_baudrate(&ftHandle, 250000) < 0)
   {
      TRACE_ERROR("FT_SetDivisor - Set baud rate error");
      return -1;
   }

   // Set the data characteristics
   if (ftdi_set_line_property(&ftHandle, BITS_8, STOP_BIT_2, NONE) < 0)
   {
      TRACE_ERROR("Set Data Characteristics error"); 
      return -1;
   }

   // Set flow control
   if (ftdi_setflowctrl(&ftHandle, SIO_DISABLE_FLOW_CTRL) < 0)
   {
      TRACE_ERROR("FT_SetFlowControl error");
      return -1;
   }

   // Set DMX PIPE for sending (clear RTS)
   if (ftdi_setrts(&ftHandle, 0) < 0)
   {
      TRACE_ERROR("Clear RTS"); 
      return -1;
   }

   // Clear TX RX buffers
   if (ftdi_usb_purge_buffers(&ftHandle) < 0)
   {
      TRACE_ERROR("Clear buffers"); 
      return -1;
   }
   
   TRACE("DMX[%d] init", dmx);
   
   return 0;
}

/** Deinitialize DMX device */
int hal_dmx_deinit(hal_dmx_t dmx)
{
   return 0;
}

/** Write buffer */
int hal_dmx_write(hal_dmx_t dmx, uint8_t *buf, int count)
{
   uint8_t start_code = 0;
   
   // Start DMX frame
   if (ftdi_set_line_property2(&ftHandle, BITS_8, STOP_BIT_2, NONE, BREAK_ON) < 0)
      return -1;
   
   if (ftdi_set_line_property2(&ftHandle, BITS_8, STOP_BIT_2, NONE, BREAK_OFF) < 0)
      return -1;

   // Start byte
   if (ftdi_write_data(&ftHandle, &start_code, 1) != 1)
      return -1;

   // Data
   if (ftdi_write_data(&ftHandle, buf, count) != count)
      return -1;
  
   return count;
}
