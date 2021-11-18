/**
 * @file lvgl_driver.c
 *
 */

#include "system.h"

#include "lvgl.h"
#include "lvgl_driver.h"
#include "lv_theme_mono.h"

#include "einkfb.h"
#include "evdev.h"

TRACE_TAG(lvgl_driver);

#ifndef CFG_LVGL_THREAD_PRIORITY
#define CFG_LVGL_THREAD_PRIORITY osPriorityNormal
#endif

#ifndef CFG_LVGL_THREAD_STACK_SIZE
#define CFG_LVGL_THREAD_STACK_SIZE 4096
#endif

#define CFG_LVGL_DISP_BUFSIZE (LV_HOR_RES_MAX * LV_VER_RES_MAX)

// Prototypes:
static void disp_driver_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static void lvgl_thread(void *arg);

// Locals:
static const osThreadDef(LVGL, lvgl_thread, CFG_LVGL_THREAD_PRIORITY, 0, CFG_LVGL_THREAD_STACK_SIZE);

static lv_disp_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t buf1[CFG_LVGL_DISP_BUFSIZE];
static lv_indev_drv_t indev_drv;

int lvgl_driver_init(void)
{
    // Initialize video framebuffer
    if (einkfb_init(CFG_EINKFB_DEVNAME) != 0)
    {
        TRACE_ERROR("einkfb init failed")
        return -1;
    }

    // Initialize display buffer
    lv_disp_buf_init(&disp_buf, buf1, NULL, CFG_LVGL_DISP_BUFSIZE);

    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize touchpad
    if (evdev_init() != 0)
    {
        TRACE_ERROR("Initialize touchscreen failed");
        return -1;
    }
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);

    // Start task
    if (osThreadCreate(osThread(LVGL), NULL) == 0)
    {
        TRACE_ERROR("Create LVGL thread failed");
        return -1;
    }

    osDelay(1000);

    TRACE("Driver init");

    return 0;
}

uint32_t lvgl_tick_get(void)
{
    return hal_time_ms();
}

static void disp_driver_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    int width = lv_area_get_width(area);
    int height = lv_area_get_height(area);
    uint32_t size = lv_area_get_size(area);
    //hal_time_t start_tm = hal_time_ms();

    // Send area to display without immediate refresh
    einkfb_write(LV_DISPLAY_ORIENTATION, area->x1, area->y1, width, height, (uint8_t *)color_map, size);
    //TRACE("DISP write  x:%d  y:%d  w:%d  h:%d  size: %d  %d ms", area->x1, area->y1, width, height, size, (int)(hal_time_ms() - start_tm));

    if (lv_disp_flush_is_last(drv)) 
        einkfb_update();

    // Indicate you are ready with the flushing
    lv_disp_flush_ready(drv);
}

static void lvgl_thread(void *arg)
{
    hal_time_t task_tmo = 0;

    TRACE("Thread started");

    while (1)
    {
        if (task_tmo <= hal_time_ms())
        {
            uint32_t next_time = lv_task_handler();
            task_tmo = hal_time_ms() + next_time;
        }

        hal_delay_ms(LV_DISP_DEF_REFR_PERIOD);
    }
}
