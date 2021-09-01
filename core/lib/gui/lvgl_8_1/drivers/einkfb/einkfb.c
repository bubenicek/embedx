
#include "system.h"

#include <math.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <errno.h>

#include "lvgl/lvgl.h"
#include "einkfb.h"

TRACE_TAG(einkfb);

#define PIXEL_WRITE 0x1 // Zapsat pixel
#define PIXEL_WIPE 0x2  // Smazat pixel
#define PIXEL_LEAVE 0x0 // Nechat pixel beze zmeny

#define DEG2RAD(angleInDegrees) ((angleInDegrees)*M_PI / 180.0)
#define RAD2DEG(angleInRadians) ((angleInRadians)*180.0 / M_PI)

// Prototypes:
void einkfb_test(void);

typedef struct
{
   int x;
   int y;
   int w;
   int h;
   const uint8_t *buf;
   int bufsize;

} eink_ioctl_draw_t;

#define EINK_IOCTL_UPDATE _IO('A', 0)
#define EINK_IOCTL_DRAW _IOR('A', 1, eink_ioctl_draw_t *)
#define EINK_IOCTL_STATUS _IOR('A', 2, int)

/** Simple frame buffer info struct */
typedef struct fb_info
{
   int fb;
   int display_size;
   uint8_t *ptr;

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

   // Clear all display
   einkfb_clear();

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

   do
   {
      if ((res = write(fb_info.fb, fb_info.ptr, fb_info.display_size)) < 0)
      {
         if (errno != 16)
         {
            TRACE_ERROR("Update fb failed");
            return -1;
         }

         hal_delay_ms(5);
      }

   } while (res != fb_info.display_size);

   return 0;
}

/** Redraw display buffer to eink display */
int einkfb_refresh(void)
{
   int res;

   do
   {
      if ((res = ioctl(fb_info.fb, EINK_IOCTL_UPDATE)) < 0)
      {
         if (errno != 16)
         {
            TRACE_ERROR("Refresh failed");
            return -1;
         }

         hal_delay_ms(5);
      }

   } while (res != 0);

   return 0;
}

/** Clear display */
int einkfb_clear(void)
{
   int res = 0;

   // Clear
   memset(fb_info.ptr, 0xAA, fb_info.display_size);
   res += einkfb_update();
   hal_delay_ms(250);

   return res;
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

   switch(orientation)
   {
      case EINKFB_ORIENTATION_0:
      {
         // No rotation
         for (row = 0; row < height; row++)
         {
            for (col = 0; col < width; col++)
               einkfb_set_pixel(col, row, *buf++);
         }
      }
      break;

      case EINKFB_ORIENTATION_90:
      {
         // Rotate 90
         for (row = 0; row < height; row++)
         {
            for (col = 0; col < width; col++)
               einkfb_set_pixel((CFG_EINKFB_HOR_RES_MAX - height - y) + height - 1 - row, x + col, *buf++);
         }
      }
      break;

      case EINKFB_ORIENTATION_180:
      {
         // Rotate 180
         for (row = 0; row < height; row++)
         {
            for (col = 0; col < width; col++)
               einkfb_set_pixel(width - 1 - col, height - 1 - row, *buf++);
         }
      }
      break;

      case EINKFB_ORIENTATION_270:
      {
         // Rotate 270
         for (row = 0; row < height; row++)
         {
            for (col = 0; col < width; col++)
               einkfb_set_pixel(y + row, (CFG_EINKFB_VER_RES_MAX - width - x) + width - 1 - col, *buf++);
         }
      }
      break;

      default:
         TRACE("Not supported orientation %d", orientation);
         return -1;
   }

   return 0;
}

void einkfb_test(void)
{
   int ix, iy;

   TRACE("Start test");

   for (iy = 0; iy < CFG_EINKFB_VER_RES_MAX; iy++)
   {
      for (ix = 0; ix < CFG_EINKFB_HOR_RES_MAX; ix++)
      {
         einkfb_set_pixel(ix, iy, 0);
      }

      einkfb_update();
   }
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


