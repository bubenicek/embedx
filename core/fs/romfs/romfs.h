

#ifndef __ROMFS_H
#define __ROMFS_H

#include <sys/types.h>
#include <sys/stat.h>

#ifndef CFG_ROMFS_MAXOPEN_FILES
#define CFG_ROMFS_MAXOPEN_FILES     4
#endif

#ifndef CFG_ENABLE_ROMFS_POSIX
#define CFG_ENABLE_ROMFS_POSIX      0
#endif


/** File data definition */
struct romfs_fsdata_file
{
   const struct romfs_fsdata_file *next;
   const unsigned char *name;
   const unsigned char *data;
   const int len;
   
} romfs_fsdata_file_t;


/** File */
typedef struct
{
   const uint8_t *data;
   uint32_t len;
	uint32_t	fptr;			   

} romfs_file_t;


/** Open file */
int romfs_open(const char *path, int flags);

/** Close file */
int romfs_close(int fd);

/** Read from file */
int romfs_read(int fd, void *buf, int count);

/** Seek to offset */
off_t romfs_lseek(int fd, off_t offset, int whence);

/** File stat */
int romfs_fstat(int fd, struct stat *buf);

/** Get romfs file by name */
struct romfs_fsdata_file *romfs_get_file(const char *filename);

/** Get next romfs file */
struct romfs_fsdata_file *romfs_next_file(struct romfs_fsdata_file *curfile);

#endif // __ROMFS_H
