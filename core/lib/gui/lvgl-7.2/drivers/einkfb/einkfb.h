

#ifndef __EINKFB_H
#define __EINKFB_H

#ifndef CFG_EINKFB_DEVNAME
#define CFG_EINKFB_DEVNAME    "/dev/eink-panel"
#endif

#ifndef CFG_EINKFB_HOR_RES_MAX
#define CFG_EINKFB_HOR_RES_MAX   1024
#endif

#ifndef CFG_EINKFB_VER_RES_MAX
#define CFG_EINKFB_VER_RES_MAX    758
#endif

#ifndef CFG_EINKFB_ORIENTATION
#define CFG_EINKFB_ORIENTATION   EINKFB_ORIENTATION_90
#endif

typedef enum
{
   EINKFB_ORIENTATION_0    = 0,
   EINKFB_ORIENTATION_90   = 90,
   EINKFB_ORIENTATION_180  = 180,
   EINKFB_ORIENTATION_270  = 270,

} einkfb_orientation_t;

/** Initialize eink driver */
int einkfb_init(const char *dev);

/** Update data in eink framebuffer and refresh display */
int einkfb_update(void);

/** Clear display */
int einkfb_clear(void);

/** Write buffer to framebuffer */
int einkfb_write(einkfb_orientation_t orientation, int x, int y, int w, int h, uint8_t *buf, int bufsize);

#endif   // __EINKFB_H