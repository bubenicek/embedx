/**
 * \file dfs.h    \brief Data FW file system interface
 */

#ifndef __DFS_H
#define __DFS_H

#include "app_header.h"

typedef enum
{
   DFS_ACTIVE = 0,
   DFS_UPGRADE,
   DFS_BACKUP

} dfs_type_t;

/** Read header */
int dfs_read_header(dfs_type_t dfs_type, app_header_t *hdr);

/** Copy data between dfs */
int dfs_copy(dfs_type_t src, dfs_type_t dst);

//
// DFS interface
//
int dfs_init(void);
int dfs_deinit(void);
int dfs_open(dfs_type_t dfs_type, app_header_t *hdr);
int dfs_close(dfs_type_t dfs_type);
int dfs_erase(dfs_type_t dfs_type);
int dfs_read(dfs_type_t dfs_type, void *buf, int bufsize);
int dfs_write(dfs_type_t dfs_type, const void *buf, int bufsize);

#endif // __DFS_H