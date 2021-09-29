/**
 * @file lvgl_driver.h
 *
 */

#ifndef LVGL_DRIVER_H
#define LVGL_DRIVER_H

int lvgl_driver_init(void);

void lvgl_lock();

void lvgl_unlock();

#endif /*LVGL_DRIVER_H*/
