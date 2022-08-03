
#include "system.h"

#include <math.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <errno.h>

#include "lvgl.h"
#include "einkfb.h"
#include "lib/crc32.h"


TRACE_TAG(einkfb);

#define CFG_EINKFB_REFRESH_CYCLES             5
#define CFG_EINKFB_CLEAR_DISPLAY_TIMEOUT      0


#define PIXEL_WRITE         0x1   // Zapsat pixel
#define PIXEL_WIPE          0x2   // Smazat pixel
#define PIXEL_LEAVE         0x0   // Nechat pixel beze zmeny

#define DEG2RAD(angleInDegrees) ((angleInDegrees)*M_PI / 180.0)
#define RAD2DEG(angleInRadians) ((angleInRadians)*180.0 / M_PI)

#define EINK_IOCTL_UPDATE _IO('A', 0)
#define EINK_IOCTL_POWERDOWN _IO('A', 1)

/** Simple frame buffer info struct */
typedef struct fb_info
{
    int fb;
    int display_size;
    uint8_t *ptr;
    uint32_t crc;

    uint8_t *ptr2;

    struct
    {
        int line_length;

    } fix;

    struct
    {
        int bits_per_pixel;
        int yres;
        int xres;

    } var;

} fbinfo;

// Locals:
static struct fb_info fb_info;


/** Initialize framebuffer driver */
int einkfb_init(const char *dev)
{
    memset(&fb_info, 0, sizeof(fb_info));
    fb_info.fb = -1;

    if ((fb_info.fb = open(dev, O_RDWR)) < 0)
    {
        TRACE_ERROR("Can not open eink fb device '%s'", dev);
        throw_exception(fail);
    }

    fb_info.var.xres = CFG_EINKFB_HOR_RES_MAX;
    fb_info.var.yres = CFG_EINKFB_VER_RES_MAX;
    fb_info.fix.line_length = fb_info.var.xres / 4;
    fb_info.var.bits_per_pixel = 2;
    fb_info.display_size = fb_info.var.xres / 4 * fb_info.var.yres;

    if ((fb_info.ptr = calloc(1, fb_info.display_size)) == NULL)
    {
        TRACE_ERROR("Can not alloc fb memory");
        throw_exception(fail);
    }

    if ((fb_info.ptr2 = calloc(1, fb_info.display_size)) == NULL)
    {
        TRACE_ERROR("Can not alloc fb memory");
        throw_exception(fail);
    }

#if CFG_EINKFB_CLEAR_DISPLAY_TIMEOUT == 0
    einkfb_clear();
#endif

    TRACE("fb device '%s' opened", dev);

    return 0;

fail:
    if (fb_info.fb != -1)
    {
        close(fb_info.fb);
        fb_info.fb = -1;
    }
    return -1;
}

/** Write framebuffer to display buffer and draw on eink display */
int einkfb_update(void)
{
    int res;
    uint32_t crc;

    crc = crc32(0, fb_info.ptr, fb_info.display_size);
    if (crc == fb_info.crc)
    {
        // Screen are the same
        return 0;
    }

    fb_info.crc = crc;

    // Write buffer
    do
    {
        if ((res = write(fb_info.fb, fb_info.ptr, fb_info.display_size)) < 0)
        {
            if (errno != 16)
            {
                TRACE_ERROR("Update fb failed");
                return -1;
            }

            hal_delay_ms(1);
        }

    } while (res != fb_info.display_size);

    // Update display
    for (int ix = 0; ix < CFG_EINKFB_REFRESH_CYCLES; ix++)
    {
        do
        {
            if ((res = ioctl(fb_info.fb, EINK_IOCTL_UPDATE)) < 0)
            {
                if (errno != 16)
                {
                    TRACE_ERROR("Refresh failed");
                    return -1;
                }

                hal_delay_ms(1);
            }

        } while (res != 0);
    }

    TRACE("%s", __FUNCTION__);

    return 0;
}

/** Set one pixel in zero display orientation */
static inline void einkfb_set_pixel(int x, int y, uint8_t state)
{
    int byte_index;
    int bit_pos;
    int bit_shift;
    int bit_mask;

    byte_index = fb_info.fix.line_length * y + (x / 4);
    bit_pos = 0x8 >> (x & 3);

    for (bit_shift = 0; bit_shift < 8; bit_shift++)
    {
        if (bit_pos & (1 << bit_shift))
        {
            bit_shift *= 2;
            break;
        }
    }

    bit_mask = (3 << bit_shift);

    if (state)
        fb_info.ptr[byte_index] = (fb_info.ptr[byte_index] & ~bit_mask) | ((PIXEL_WIPE << bit_shift) & bit_mask);
    else
        fb_info.ptr[byte_index] = (fb_info.ptr[byte_index] & ~bit_mask) | ((PIXEL_WRITE << bit_shift) & bit_mask);
}

/** Clear display */
int einkfb_clear(void)
{
    // Black
    for (int row = 0; row < CFG_EINKFB_VER_RES_MAX; row++)
    {
        for (int col = 0; col < CFG_EINKFB_HOR_RES_MAX; col++)
            einkfb_set_pixel(col, row, 0x00);
    }
    einkfb_update();

    return 0;
}

int einkfb_clear_region(einkfb_orientation_t orientation, int x, int y, int width, int height)
{
    int row, col;

    switch (orientation)
    {
        case EINKFB_ORIENTATION_0:
        {        
            // No rotation
            for (row = 0; row < height; row++)
            {
            //    for (col = 0; col < width; col++, pbuf++)
            //        einkfb_set_pixel(col, row, lv_color_to1(*pbuf));
            }
        }
        break;

        case EINKFB_ORIENTATION_90:
        {
            // Rotate 90

            // Black
            for (row = 0; row < height; row++)
            {
                for (col = 0; col < width; col++)
                    einkfb_set_pixel((CFG_EINKFB_HOR_RES_MAX - height - y) + height - 1 - row, x + col, 0x00);
            }
            einkfb_update();

            // White
            for (row = 0; row < height; row++)
            {
                for (col = 0; col < width; col++)
                    einkfb_set_pixel((CFG_EINKFB_HOR_RES_MAX - height - y) + height - 1 - row, x + col, 0xFF);
            }
            einkfb_update();
        }
        break;

        case EINKFB_ORIENTATION_180:
        {
            // Rotate 180
            for (row = 0; row < height; row++)
            {
            //    for (col = 0; col < width; col++, pbuf++)
            //        einkfb_set_pixel(width - 1 - col, height - 1 - row, lv_color_to1(*pbuf));
            }
        }
        break;

        case EINKFB_ORIENTATION_270:
        {
            // Rotate 270
            for (row = 0; row < height; row++)
            {
            //    for (col = 0; col < width; col++, pbuf++)
            //        einkfb_set_pixel(y + row, (CFG_EINKFB_VER_RES_MAX - width - x) + width - 1 - col, lv_color_to1(*pbuf));
            }
        }
        break;

        default:
            TRACE("Not supported orientation %d", orientation);
            return -1;
    }

    return 0;
}

/*
 * Write image buffer at position
 * 
 * Bitmap rotation example:
 * ------------------------
 * image[y][x]                                        assuming this is the original orientation
 * image[x][original_width - y]                       rotated 90 degrees ccw
 * image[original_height - x][y]                      90 degrees cw 
 * image[original_height - y][original_width - x]     180 degrees 
 */
int einkfb_write(einkfb_orientation_t orientation, int x, int y, int width, int height, uint8_t *buf, int bufsize)
{
    int row, col;
    lv_color_t *pbuf = (lv_color_t *)buf;

#if CFG_EINKFB_CLEAR_DISPLAY_TIMEOUT > 0
    static hal_time_t clear_tmo = 0;
    if (hal_time_ms() >= clear_tmo)
    {  
//        memcpy(fb_info.ptr2, fb_info.ptr, fb_info.display_size);
//        einkfb_clear();
//        memcpy(fb_info.ptr, fb_info.ptr2, fb_info.display_size);
     
        einkfb_clear_region(orientation, x, y, width, height);

        clear_tmo = hal_time_ms() + CFG_EINKFB_CLEAR_DISPLAY_TIMEOUT;
    }
#endif    

    //TRACE("%s   x:%d  y:%d  w:%d  h:%d", __FUNCTION__, x, y, width, height);

    switch (orientation)
    {
        case EINKFB_ORIENTATION_0:
        {
            // No rotation
            for (row = 0; row < height; row++)
            {
                for (col = 0; col < width; col++, pbuf++)
                    einkfb_set_pixel(col, row, lv_color_to1(*pbuf));
            }
        }
        break;

        case EINKFB_ORIENTATION_90:
        {
            // Rotate 90
            for (row = 0; row < height; row++)
            {
                for (col = 0; col < width; col++, pbuf++)
                    einkfb_set_pixel((CFG_EINKFB_HOR_RES_MAX - height - y) + height - 1 - row, x + col, lv_color_to1(*pbuf));
            }
        }
        break;

        case EINKFB_ORIENTATION_180:
        {
            // Rotate 180
            for (row = 0; row < height; row++)
            {
                for (col = 0; col < width; col++, pbuf++)
                    einkfb_set_pixel(width - 1 - col, height - 1 - row, lv_color_to1(*pbuf));
            }
        }
        break;

        case EINKFB_ORIENTATION_270:
        {
            // Rotate 270
            for (row = 0; row < height; row++)
            {
                for (col = 0; col < width; col++, pbuf++)
                    einkfb_set_pixel(y + row, (CFG_EINKFB_VER_RES_MAX - width - x) + width - 1 - col, lv_color_to1(*pbuf));
            }
        }
        break;

        default:
            TRACE("Not supported orientation %d", orientation);
            return -1;
    }

    return 0;
}

#if 0
/** Draw monochrome buffer (1Byte per pixel) to framebuffer (2bits per pixel) */
int einkfb_write(int x, int y, int w, int h, uint8_t *buf, int bufsize)
{
   int ix, iy;
 	int byte_index, bit_pos, bit_shift, bit_mask;

   for (iy = y; iy < y + h; iy++)
   {
      for (ix = x; ix < x + w; ix++)
      {
         byte_index = fb_info.fix.line_length * iy + (ix/4);		
         bit_pos = 0x8 >> (ix & 3);										

         for (bit_shift = 0; bit_shift < 8; bit_shift++) 
         {
            if (bit_pos & ( 1 << bit_shift)) 
            {
               bit_shift *= 2;
               break;
            }
         }

         bit_mask = (3 << bit_shift);

         if (*buf++) 
            fb_info.ptr[byte_index] = (fb_info.ptr[byte_index] & ~bit_mask) | ((PIXEL_WIPE << bit_shift) & bit_mask);
         else 
            fb_info.ptr[byte_index] = (fb_info.ptr[byte_index] & ~bit_mask) | ((PIXEL_WRITE << bit_shift) & bit_mask);       
      }
   }

   return 0;
}
#endif
