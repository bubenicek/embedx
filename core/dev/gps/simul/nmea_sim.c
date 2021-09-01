
#include "system.h"

TRACE_TAG(gps_sim);

// Locals:

/** Initialize GPS driver */
int gps_driver_init(void)
{
	TRACE("Init");
   return 0;
}

/** Read NMEA data from GPS driver */
int gps_driver_read(char *buf, int bufsize)
{
   return 0;
}

