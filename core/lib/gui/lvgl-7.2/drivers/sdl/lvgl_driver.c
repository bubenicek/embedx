#include "system.h"

#include <unistd.h>
#define SDL_MAIN_HANDLED        /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "display/sdl_monitor.h"
#include "indev/sdl_mouse.h"
#include "indev/sdl_mousewheel.h"
#include "indev/sdl_keyboard.h"

TRACE_TAG(lvgl_driver);

#define CFG_LVGL_DISP_BUFSIZE   			(LV_HOR_RES_MAX * LV_VER_RES_MAX)

static lv_disp_drv_t disp_drv;
static lv_disp_buf_t disp_buf;
static lv_color_t buf[CFG_LVGL_DISP_BUFSIZE];               /*Declare a buffer for 10 lines*/
static lv_indev_drv_t indev_drv;

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void * data)
{
    (void)data;

    while(1) 
    {
        lv_task_handler();
        SDL_Delay(20);   /*Sleep for 5 millisecond*/
        //lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

int lvgl_driver_init(void)
{
    // Workaround for sdl2 `-m32` crash
    // https://bugs.launchpad.net/ubuntu/+source/libsdl2/+bug/1775067/comments/7
    #ifndef WIN32
        setenv("DBUS_FATAL_WARNINGS", "0", 1);
    #endif

    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    sdl_monitor_init();

    lv_disp_buf_init(&disp_buf, buf, NULL, CFG_LVGL_DISP_BUFSIZE);    /*Initialize the display buffer*/

    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.flush_cb = sdl_monitor_flush;    /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    disp_drv.buffer = &disp_buf;
    //disp_drv.disp_fill = monitor_fill;      /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    //disp_drv.disp_map = monitor_map;        /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    lv_disp_drv_register(&disp_drv);

    /* Add the mouse as input device
     * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_drv_register(&indev_drv);

    /* Tick init.
     * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about how much time were elapsed
     * Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);
    osDelay(250);

    TRACE("Init, display resolution: %dx%d", LV_HOR_RES_MAX, LV_VER_RES_MAX);

    return 0;
}

int lvgl_driver_deinit(void)
{
    return 0;
}


uint32_t lvgl_tick_get(void)
{
	return hal_time_ms();
}

