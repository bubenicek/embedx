
PROJECT_DIRS += $(EMBEDX_ROOT)/core/lib/gui/lvgl/drivers/sdl
PROJECT_DIRS += $(EMBEDX_ROOT)/core/lib/gui/lvgl/drivers/sdl/display
PROJECT_DIRS += $(EMBEDX_ROOT)/core/lib/gui/lvgl/drivers/sdl/indev

PROJECT_SOURCEFILES += lvgl_driver.c
PROJECT_SOURCEFILES += sdl_monitor.c
PROJECT_SOURCEFILES += sdl_keyboard.c
PROJECT_SOURCEFILES += sdl_mouse.c
PROJECT_SOURCEFILES += sdl_mousewheel.c

LIBS += -lSDL2
