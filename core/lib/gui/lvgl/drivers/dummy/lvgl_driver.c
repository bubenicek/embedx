/**
 * @file lvgl_driver.c
 *
 */

#include "system.h"

#include "lvgl/lvgl.h"
#include "lvgl_driver.h"
#include "lv_theme_mono.h"

TRACE_TAG(lvgl_driver);

#ifndef CFG_LVGL_THREAD_PRIORITY
#define CFG_LVGL_THREAD_PRIORITY 		osPriorityNormal
#endif

#ifndef CFG_LVGL_THREAD_STACK_SIZE
#define CFG_LVGL_THREAD_STACK_SIZE		4096
#endif

#ifndef CFG_LVGL_THREAD_PERIOD
#define CFG_LVGL_THREAD_PERIOD			5
#endif

#define CFG_LVGL_DISP_BUFSIZE   			(LV_HOR_RES_MAX * 300)


// Prototypes:
static void disp_driver_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);
static void disp_driver_rounder(lv_disp_drv_t * disp_drv, lv_area_t * area);
static void lvgl_thread(void *arg);

// Locals:
static const osThreadDef(LVGL, lvgl_thread, CFG_LVGL_THREAD_PRIORITY, 0, CFG_LVGL_THREAD_STACK_SIZE);

static lv_disp_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t buf1[CFG_LVGL_DISP_BUFSIZE];


int lvgl_driver_init(void)
{
	// Initialize display buffer
	lv_disp_buf_init(&disp_buf, buf1, NULL, CFG_LVGL_DISP_BUFSIZE);

	// Initialize driver
	lv_disp_drv_init(&disp_drv);

   disp_drv.flush_cb = disp_driver_flush;
	disp_drv.rounder_cb = disp_driver_rounder;
#ifdef CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
   disp_drv.set_px_cb = disp_driver_set_px;
#endif
   disp_drv.buffer = &disp_buf;
   lv_disp_drv_register(&disp_drv);

	// Start task
   if (osThreadCreate(osThread(LVGL), NULL) == 0)
	{
		TRACE_ERROR("Create LVGL thread failed");
		return -1;
	}

	TRACE("Driver init");

	return 0;
}

uint32_t lvgl_tick_get(void)
{
	return hal_time_ms();
}

static void disp_driver_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
	int width = lv_area_get_width(area);
	int height = lv_area_get_height(area);
   uint32_t size = lv_area_get_size(area);
	hal_time_t start_tm;

	for (int ix = 0; ix < size; ix++)
	{
		if (color_map[ix].full != 0xFF)
			color_map[ix].full = 0;
	}

	// Send area to display
	start_tm = hal_time_ms();
	//IT8951_writeBitmap((uint8_t *)color_map, area->x1, area->y1, width, height, IT8951_MODE_1);
	TRACE("DISP write  x:%d  y:%d  w:%d  h:%d   %d ms", area->x1, area->y1, width, height, (int)(hal_time_ms() - start_tm));

	// Indicate you are ready with the flushing
 	lv_disp_flush_ready(drv);         
}

static void disp_driver_rounder(lv_disp_drv_t * disp_drv, lv_area_t * area)
{
	if (lv_area_get_width(area) & 0x1)
		area->x2++;

	if (lv_area_get_height(area) & 0x1)
		area->y2++;
}

static void lvgl_thread(void *arg)
{
	TRACE("Thread started");

	while(1)
	{
		lv_task_handler();
		osDelay(CFG_LVGL_THREAD_PERIOD);
	}
}
