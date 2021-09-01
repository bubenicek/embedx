
#ifndef __GPS_DRIVER_H
#define __GPS_DRIVERS_H

/** Initialize GPS driver */
int gps_driver_init(void);

/** Deinitialize GPS driver */
int gps_driver_deinit(void);

/** Read NMEA data from GPS driver */
int gps_driver_read(char *buf, int bufsize);

#endif  // __GPS_DRIVERS_H