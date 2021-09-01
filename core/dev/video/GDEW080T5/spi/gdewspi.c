
#include "system.h"
#include "gdewspi.h"

TRACE_TAG(gdewspi);

#define SEND_CHUNK_SIZE    1024
#define BUSY_TMO           5000


// Prototypes:

#define GDEW_CHECK_BUSY() do {                                       \
   hal_time_t busy_tmo = hal_time_ms() + BUSY_TMO;                   \
   while(hal_gpio_get(GDEW_BUSY) && hal_time_ms() < busy_tmo) {      \
      hal_delay_ms(10);                                              \
   }                                                                 \
   if (hal_time_ms() >= busy_tmo) {                                  \
      TRACE_ERROR("gdew is busy");                                   \
      return -1;                                                     \
   };                                                                \
} while(0)


// Locals:
static hal_spi_t gdew_spi;


/** Initialize gdewspi interface */
int gdewspi_init(hal_spi_t spi)
{
   gdew_spi = spi;

   if (hal_spi_init(gdew_spi) != 0)
      return -1;

   TRACE("GDEW SPI init")
   
   return 0;
}

/** Write to control register */
int gdewspi_write_ctrl(uint8_t reg)
{
   gdew_spi_frame_t frame = {
      .cmd = GDEW_CMD_CONTROL,
      .control.reg = reg
   };

   return hal_spi_write(gdew_spi, (uint8_t *)&frame, GDEW_SPI_FRAMESIZE(gdew_cmd_control_t));
}

/** Read status register */
int gdewspi_read_status(void)
{
   return 0;
}

/** Clear display */
int gdewspi_clear_display(void)
{
   gdew_spi_frame_t frame = {
      .cmd = GDEW_CMD_CLEAR
   };

   GDEW_CHECK_BUSY();

   TRACE("Clear display");

   return hal_spi_write(gdew_spi, (uint8_t *)&frame, GDEW_SPI_FRAMESIZE(gdew_cmd_clear_t));
}

/** Draw gray shape */
int gdewspi_draw_shape(uint8_t gray, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
   gdew_spi_frame_t frame = {
      .cmd = GDEW_CMD_DRAW_SHAPE,
      .draw_shape.gray = gray,
      .draw_shape.x = x,
      .draw_shape.y = y,
      .draw_shape.width = width,
      .draw_shape.height = height
   };

   GDEW_CHECK_BUSY();

   TRACE("Draw_shape - gray:%d  x:%d y:%d w:%d h:%d", gray, x, y, width, height);

   return hal_spi_write(gdew_spi, (uint8_t *)&frame, GDEW_SPI_FRAMESIZE(gdew_cmd_draw_shape_t));
}

int gdewspi_draw_bitmap(const uint8_t *bitmap, int size, uint8_t bpp, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
   int res, len;

   gdew_spi_frame_t frame = {
      .cmd = GDEW_CMD_DRAW_BITMAP,
      .draw_bitmap.bpp = bpp,
      .draw_bitmap.x = x,
      .draw_bitmap.y = y,
      .draw_bitmap.width = width,
      .draw_bitmap.height = height,
      .draw_bitmap.size = size
   };

   GDEW_CHECK_BUSY();

   TRACE("Draw_bitmap - size:%d  bpp:%d  x:%d y:%d w:%d h:%d", size, bpp, x, y, width, height);

   // Write header
   if (hal_spi_write_burst(gdew_spi, (uint8_t *)&frame, GDEW_SPI_FRAMESIZE(gdew_cmd_draw_bitmap_t), false) < 0)
      throw_exception(fail);

   // Write bitmap data
   while(size > 0)
   {
      len = (size > SEND_CHUNK_SIZE) ? SEND_CHUNK_SIZE : size;

      if ((res = hal_spi_write_burst(gdew_spi, bitmap, len, (size - len) <= 0)) < 0)
      {
         TRACE_ERROR("Write bitmap to spi failed");
         throw_exception(fail);
      }

      bitmap += res;
      size -= res;
   }

   return 0;

fail:
   return -1;
}

/** Refresh drawd bitmaps */
int gdewspi_refresh_display(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
   gdew_spi_frame_t frame = {
      .cmd = GDEW_CMD_REFRESH,
      .refresh.x = x,
      .refresh.y = y,
      .refresh.width = width,
      .refresh.height = height
   };

   GDEW_CHECK_BUSY();

   TRACE("Refresh - x:%d y:%d w:%d h:%d", x, y, width, height);

   return hal_spi_write(gdew_spi, (uint8_t *)&frame, GDEW_SPI_FRAMESIZE(gdew_cmd_refresh_t));
}


/** Send test transmit packet */
int gdewspi_test_transmit(uint8_t *buf, int bufsize)
{
  gdew_spi_frame_t frame = {
      .cmd = GDEW_CMD_TEST,
      .test.size = bufsize
   };

   GDEW_CHECK_BUSY();

   if (hal_spi_write_burst(gdew_spi, (uint8_t *)&frame, GDEW_SPI_FRAMESIZE(gdew_cmd_test_t), false) < 0)
      return -1;

   return hal_spi_write_burst(gdew_spi, buf, bufsize, true);
}
