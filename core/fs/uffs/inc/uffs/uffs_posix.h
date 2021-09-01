
#ifndef __UFFS_POSIX_H
#define __UFFS_POSIX_H

#include "uffs/uffs_fd.h"

typedef long off_t;

#define	SEEK_SET	   USEEK_SET
#define	SEEK_CUR	   USEEK_CUR
#define	SEEK_END    USEEK_END

#define O_EXCL       UO_EXCL
#define O_CREAT      UO_CREATE
#define O_RDONLY     UO_RDONLY
#define O_WRONLY     UO_WRONLY
#define O_RDWR       UO_RDWR

#define open         uffs_open
#define close        uffs_close
#define read         uffs_read
#define write        uffs_write
#define lseek        uffs_seek
#define tell         uffs_tell
#define eof          uffs_eof
#define flush        uffs_flush
#define rename       uffs_rename
#define unlink       uffs_remove
#define truncate     uffs_ftruncate
#define fsync(_fd)   ({int __ret = 0; uffs_flush_all("/"); __ret;})

#define mkdir        uffs_mkdir
#define rmdir        uffs_rmdir

#define lstat        uffs_lstat
#define fstat        uffs_fstat

#define closedir     uffs_closedir
#define opendir      uffs_opendir
#define dirent       uffs_dirent

#define rewinddir    uffs_rewinddir


#endif // __UFFS_POSIX_H
