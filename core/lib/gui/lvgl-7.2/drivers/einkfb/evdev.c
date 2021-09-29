/**
 * @file evdev.c
 *
 */

#include "system.h"
#include "evdev.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>

TRACE_TAG(evdev);

// Prototypes:
static int map(int x, int in_min, int in_max, int out_min, int out_max);

// Locals:
static int evdev_fd;
static int evdev_root_x;
static int evdev_root_y;
static int evdev_button;
static int evdev_key_val;
static int display_orientation = LV_DISPLAY_ORIENTATION;

#define calib_enabled() ( \
            app_config.gui.touchpad.calibration.x1 > 0 && app_config.gui.touchpad.calibration.x2 && \
            app_config.gui.touchpad.calibration.y1 > 0 && app_config.gui.touchpad.calibration.y2)

/**
 * Initialize the evdev interface
 */
int evdev_init(void)
{
    if ((evdev_fd = open(EVDEV_NAME, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
    {
        TRACE_ERROR("unable open evdev interface");
        return -1;
    }

    if (fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK) < 0)
        return -1;

    evdev_root_x = 0;
    evdev_root_y = 0;
    evdev_key_val = 0;
    evdev_button = LV_INDEV_STATE_REL;

    TRACE("evdev init, device: %s", EVDEV_NAME);
    TRACE("  Calibration %s - x1:%d  x2:%d  y1:%d  y2:%d  offset: %d  orientation: %d", 
        calib_enabled() ? "enabled" : "disabled",
        app_config.gui.touchpad.calibration.x1,
        app_config.gui.touchpad.calibration.x2,
        app_config.gui.touchpad.calibration.y1,
        app_config.gui.touchpad.calibration.y2, 
        app_config.gui.touchpad.calibration.offset,
        display_orientation);

    return 0;
}

/** Set calibration */
int evdev_set_calibration(int x1, int x2, int y1, int y2, int offset)
{
    app_config.gui.touchpad.calibration.x1 = x1;
    app_config.gui.touchpad.calibration.x2 = x2;
    app_config.gui.touchpad.calibration.y1 = y1;
    app_config.gui.touchpad.calibration.y2 = y2;
    app_config.gui.touchpad.calibration.offset = offset;

    if (x1 != 0 && x2 != 0 && y1 != 0 && y2 != 0)
    {
        app_config.gui.touchpad.calibration.enabled = true;

        TRACE("Set calibration  x1:%d  x2:%d  y1:%d  y2:%d  offset: %d", x1, x2, y1, y2, offset);

        if (config_save() != 0)
        {
            TRACE_ERROR("Save config failed");
            return -1;
        }
    }
    else
    {
        TRACE("Disable calibration");
    }

    return 0;
}

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool evdev_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    static int prev_x = 0;
    static int prev_y = 0;
    struct input_event in;

    while (read(evdev_fd, &in, sizeof(struct input_event)) > 0)
    {
        if (in.type == EV_REL)
        {
            if (in.code == REL_X)
                evdev_root_x += in.value;
            else if (in.code == REL_Y)
                evdev_root_y += abs(in.value + EVDEV_VER_RES_CALIB);
        }
        else if (in.type == EV_ABS)
        {
            if (in.code == ABS_X)
                evdev_root_x = in.value;
            else if (in.code == ABS_Y)
                evdev_root_y = abs(in.value + EVDEV_VER_RES_CALIB);
            else if (in.code == ABS_MT_POSITION_X)
                evdev_root_x = in.value;
            else if (in.code == ABS_MT_POSITION_Y)
                evdev_root_y = abs(in.value + EVDEV_VER_RES_CALIB);
        }
        else if (in.type == EV_KEY)
        {
            if (in.code == BTN_MOUSE || in.code == BTN_TOUCH)
            {
                if (in.value == 0)
                    evdev_button = LV_INDEV_STATE_REL;
                else if (in.value == 1)
                    evdev_button = LV_INDEV_STATE_PR;
            }
            else if (drv->type == LV_INDEV_TYPE_KEYPAD)
            {
                data->state = (in.value) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
                switch (in.code)
                {
                case KEY_BACKSPACE:
                    data->key = LV_KEY_BACKSPACE;
                    break;
                case KEY_ENTER:
                    data->key = LV_KEY_ENTER;
                    break;
                case KEY_UP:
                    data->key = LV_KEY_UP;
                    break;
                case KEY_LEFT:
                    data->key = LV_KEY_PREV;
                    break;
                case KEY_RIGHT:
                    data->key = LV_KEY_NEXT;
                    break;
                case KEY_DOWN:
                    data->key = LV_KEY_DOWN;
                    break;
                default:
                    data->key = 0;
                    break;
                }
                evdev_key_val = data->key;
                evdev_button = data->state;
                return false;
            }
        }
    }

    if (drv->type == LV_INDEV_TYPE_KEYPAD)
    {
        /* No data retrieved */
        data->key = evdev_key_val;
        data->state = evdev_button;
        return false;
    }
    if (drv->type != LV_INDEV_TYPE_POINTER)
        return false;    
       
    // Store the collected data
    data->state = evdev_button;

    // Transform coordinates
    switch(display_orientation)
    {
        case 90:
        {
            // Rotate 90
            data->point.x = evdev_root_y;
            data->point.y = abs(EVDEV_HOR_RES - evdev_root_x);
        }
        break;

        case 270:
        {
            // Rotate 270
            data->point.x = evdev_root_y;
            data->point.y = evdev_root_x;
        }
        break;

        default:
            // No rotation
            data->point.x = evdev_root_x;
            data->point.y = evdev_root_y;
    }

    if (calib_enabled()) 
    {
        data->point.x = map(data->point.x, app_config.gui.touchpad.calibration.x1, app_config.gui.touchpad.calibration.x2, app_config.gui.touchpad.calibration.offset, lv_disp_get_hor_res(drv->disp));
        data->point.y = map(data->point.y, app_config.gui.touchpad.calibration.y1+app_config.gui.touchpad.calibration.offset, app_config.gui.touchpad.calibration.y2+app_config.gui.touchpad.calibration.offset, 0, lv_disp_get_ver_res(drv->disp));

        if (data->point.x < 0)
            data->point.x = 0;
        if (data->point.y < 0)
            data->point.y = 0;
        if (data->point.x >= lv_disp_get_hor_res(drv->disp))
            data->point.x = lv_disp_get_hor_res(drv->disp) - 1;
        if (data->point.y >= lv_disp_get_ver_res(drv->disp))
            data->point.y = lv_disp_get_ver_res(drv->disp) - 1;
    }

    if (data->point.x != prev_x || data->point.y != prev_y) 
    {
        TRACE("event x:%d  y:%d", data->point.x, data->point.y);
        prev_x = data->point.x;
        prev_y = data->point.y;
    }

    return false;
}

static int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
