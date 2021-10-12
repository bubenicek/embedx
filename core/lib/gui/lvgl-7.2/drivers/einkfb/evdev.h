/**
 * @file evdev.h
 *
 */

#ifndef EVDEV_H
#define EVDEV_H

#include "lvgl.h"


#ifndef EVDEV_HOR_RES  
#define EVDEV_HOR_RES      1919 
#endif

#ifndef EVDEV_VER_RES
#define EVDEV_VER_RES      1435
#endif

// Offset how much from Y edge
#ifndef EVDEV_VER_RES_CALIB 
#define EVDEV_VER_RES_CALIB  -500
#endif


/**
 * Initialize the evdev
 */
int evdev_init(void);

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool evdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data);

/** Set calibration */
int evdev_set_calibration(int x1, int x2, int y1, int y2, int offset);

#define evdev_disable_calibration() evdev_set_calibration(0,0,0,0,0)

#endif /* EVDEV_H */
