#ifndef __GDEW_PROT_H
#define __GDEW_PROT_H

/**
 * Control register
 */
#define GDEW_CMD_CONTROL   0xA0 
typedef struct
{
#define GDEW_CTRL_POWEN    0x01        // Enable/disable display power
   uint8_t reg;
} __PACKED__ gdew_cmd_control_t;


/**
 * Status register
 */
#define GDEW_CMD_STATUS    0xA1 
typedef struct
{
   uint8_t reg;
} __PACKED__ gdew_cmd_status_t;


/**
* Display clear
*/
#define GDEW_CMD_CLEAR     0xA2
typedef struct
{
} __PACKED__ gdew_cmd_clear_t;


/**
* Display refresh
*/
#define GDEW_CMD_REFRESH   0xA3
typedef struct
{
   uint16_t x;
   uint16_t y;
   uint16_t width;
   uint16_t height;

} __PACKED__ gdew_cmd_refresh_t;


/**
 * Draw gray shape (point, line, filled rectangle)
 */ 
#define GDEW_CMD_DRAW_SHAPE   0xA4
typedef struct
{
   uint8_t gray;
   uint16_t x;
   uint16_t y;
   uint16_t width;
   uint16_t height;

} __PACKED__ gdew_cmd_draw_shape_t;


/**
 * Draw bitmap
 */
#define GDEW_CMD_DRAW_BITMAP  0xA5
typedef struct
{
#define GDEW_2BPP           (0<<4)       	// 1 pixel occupies 2 bits, one byte represents 4 pixels
#define GDEW_4BPP           (2<<4)       	// 1 pixel occupies 4 bits, one byte represents 2 pixels
#define GDEW_8BPP           (3<<4)       	// 1 pixel occupies 8 bits, one byte represents 1 pixels
   uint8_t bpp;    
   uint16_t x;
   uint16_t y;
   uint16_t width;
   uint16_t height;
   uint32_t size;
   uint8_t data[0];

} __PACKED__ gdew_cmd_draw_bitmap_t;


/**
 * Test transfer speed
 */
#define GDEW_CMD_TEST        0xA6
typedef struct
{
   uint16_t size;

} __PACKED__ gdew_cmd_test_t;



/** SPI frame */
typedef struct
{
   uint8_t cmd;

   union
   {
      gdew_cmd_control_t control;
      gdew_cmd_status_t status;
      gdew_cmd_draw_shape_t draw_shape;
      gdew_cmd_draw_bitmap_t draw_bitmap;
      gdew_cmd_refresh_t refresh;
      gdew_cmd_test_t test;
   };

} __PACKED__ gdew_spi_frame_t;

/** Get size of SPI frame */
#define GDEW_SPI_FRAMESIZE(_elem) (1 + sizeof(_elem))



/** Initialize gdewspi interface */
int gdewspi_init(hal_spi_t spi);

/** Write to control register */
int gdewspi_write_ctrl(uint8_t reg);

/** Read status register */
int gdewspi_read_status();

/** Clear display */
int gdewspi_clear_display();

/** Draw gray shape */
int gdewspi_draw_shape(uint8_t gray, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/** Draw gray bitmap */
int gdewspi_draw_bitmap(const uint8_t *bitmap, int size, uint8_t bpp, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/** Send test buffer */
int gdewspi_test_transmit(uint8_t *buf, int bufsize);

/** Refresh drawd bitmaps */
int gdewspi_refresh_display(uint16_t x, uint16_t y, uint16_t width, uint16_t height);


#endif   // __GDEW_PROT_H