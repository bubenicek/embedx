

#ifndef __UFFS_INTERFACE_H
#define __UFFS_INTERFACE_H


/** Initialize (mount) filesystem */
int uffs_init_filesystem(void);

/** Unmount filesystem */
int uffs_deinit_filesystem(void);

/** Erase filesystem */
int uffs_erase_filesystem(void);


#endif

