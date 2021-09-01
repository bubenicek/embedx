
#ifndef __SPIFFS_DRIVER_H
#define __SPIFFS_DRIVER_H

/** Initialize (mount) filesystem */
int spiffs_mount(spiffs **fs);

/** Unmount filesystem */
int spiffs_unmount(spiffs *fs);

/** Erase filesystem */
int spiffs_format(spiffs *fs);

#endif // __SPIFFS_DRIVER_H
