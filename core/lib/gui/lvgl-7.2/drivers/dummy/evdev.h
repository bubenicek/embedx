
#ifndef __EVDEV_H
#define __EVDEV_H


/** Set calibration */
static inline int evdev_set_calibration(int x1, int x2, int y1, int y2, int offset)
{
   return 0;
}

#define evdev_disable_calibration() evdev_set_calibration(0,0,0,0,0)


#endif   // __EVDEV_H