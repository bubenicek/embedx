/**
 * @file lvgl_driver.c
 *
 */

#include "system.h"

#include "gdewspi.h"

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
#define CFG_LVGL_THREAD_PERIOD			25
#endif

#define CFG_LVGL_DISP_BUFSIZE   			((LV_HOR_RES_MAX * LV_VER_RES_MAX) / 16)


// Prototypes:
static void disp_driver_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);
static void disp_driver_rounder(lv_disp_drv_t * disp_drv, lv_area_t * area);
static void disp_monitor_cb(struct _disp_drv_t * disp_drv, uint32_t time, uint32_t px);
static void lvgl_thread(void *arg);

// Locals:
static const osThreadDef(LVGL, lvgl_thread, CFG_LVGL_THREAD_PRIORITY, 0, CFG_LVGL_THREAD_STACK_SIZE);

static lv_disp_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t buf1[CFG_LVGL_DISP_BUFSIZE];
static lv_color_t buf2[CFG_LVGL_DISP_BUFSIZE];
static lv_area_t refresh_area;
static bool refresh_area_empty = true;


int lvgl_driver_init(void)
{
	// Initialize SPI
   if (gdewspi_init(GDEW_SPI) != 0)
	{
		TRACE_ERROR("Init gdew spi failed");
		return -1;
	}

   gdewspi_clear_display();
   hal_delay_ms(1000);

	// Initialie LVGL GUI
	lv_init();

	// Initialize display buffer
	lv_disp_buf_init(&disp_buf, buf1, buf2, CFG_LVGL_DISP_BUFSIZE);

	// Initialize driver
	lv_disp_drv_init(&disp_drv);

   disp_drv.flush_cb = disp_driver_flush;
	disp_drv.rounder_cb = disp_driver_rounder;
	disp_drv.monitor_cb = disp_monitor_cb;
#ifdef CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
   disp_drv.set_px_cb = disp_driver_set_px;
#endif
   disp_drv.buffer = &disp_buf;
   lv_disp_drv_register(&disp_drv);

	// Set theme
   lv_theme_mono_init(0, NULL);
   lv_theme_set_current(lv_theme_get_mono());

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

	//for (int ix = 0; ix < size; ix++)
	//{
	//	if (color_map[ix].full != 0xFF)
	//		color_map[ix].full = 0;
	//}

#if DUMP_BITMAP
	static int count = 0;
	char path[255];
	sprintf(path, "image%d_%dx%d.raw", count++, width, height);
	FILE *fw = fopen(path, "wb");
	fwrite(color_map, sizeof(char), size, fw);
	fclose(fw);
#endif

	if (refresh_area_empty)
	{
		// Set new refresh area
		memcpy(&refresh_area, area, sizeof(lv_area_t));
		refresh_area_empty = false;
		//TRACE("SET refresh area x:%d,%d  y:%d,%d", refresh_area.x1, refresh_area.x2, refresh_area.y1, refresh_area.y2);
	}
	else
	{
		// Update refresh area
		if (area->x1 < refresh_area.x1)
			refresh_area.x1 = area->x1;

		if (area->x2 > refresh_area.x2)
			refresh_area.x2 = area->x2;

		if (area->y1 < refresh_area.y1)
			refresh_area.y1 = area->y1;

		if (area->y2 > refresh_area.y2)
			refresh_area.y2 = area->y2;

		//TRACE("UPDATE refresh area x:%d,%d  y:%d,%d", refresh_area.x1, refresh_area.x2, refresh_area.y1, refresh_area.y2);
	}

	// Send area to display
	gdewspi_draw_bitmap((uint8_t *)color_map, size, GDEW_8BPP, area->x1, area->y1, width, height);

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

static void disp_monitor_cb(struct _disp_drv_t * disp_drv, uint32_t time, uint32_t px)
{
	//gdewspi_refresh_display(refresh_area.x1, refresh_area.y1, lv_area_get_width(&refresh_area), lv_area_get_height(&refresh_area));
	//refresh_area_empty = true;
	//TRACE("Refresh time: %d  px: %d", time, px);
}

/*
void disp_driver_set_px(struct _disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
	// Write to the buffer as required for the display.
 	// Write only 1-bit for monochrome displays mapped vertically:
	buf += buf_w * (y >> 3) + x;
	if (lv_color_brightness(color) > 128) 
		(*buf) |= (1 << (y % 8));
	else 
		(*buf) &= ~(1 << (y % 8));	
}
*/

static void lvgl_thread(void *arg)
{
	TRACE("Thread started");

	while(1)
	{
		lv_task_handler();
		osDelay(CFG_LVGL_THREAD_PERIOD);
	}
}
