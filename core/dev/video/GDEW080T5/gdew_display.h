
#ifndef __GDEW_DISPLAY_H
#define __GDEW_DISPLAY_H

#include "data.h"
#include "epd.h"
#include "power.h"
#include "spi/gdewspi.h"

int gdew_display_init(void);
int gdew_display_test(void);

#endif   // __GDEW_DISPLAY_H