
#ifndef __GPSMON_H
#define __GPSMON_H


/** Initialize GPS socket monitor */
int gpsmon_init(void);

/** Send buffer to open socket if it exists*/
int gpsmon_send(uint8_t *buf, int bufsize);


#endif   // __GPSMON_H