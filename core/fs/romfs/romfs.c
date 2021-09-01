
#include "system.h"

#if defined(CFG_ENABLE_ROMFS) && (CFG_ENABLE_ROMFS == 1)

#include "romfs.h"
TRACE_TAG(romfs);

// include file system application data
#include "romfs_fsdata.c"

// Prototypes:
static inline uint8_t romfs_strcmp(const char *str1, const char *str2);

// Locals:
static romfs_file_t romfiles[CFG_ROMFS_MAXOPEN_FILES];

/** Open file */
int romfs_open(const char *path, int flags)
{
#if defined(ROMFS_NUMFILES) &&  (ROMFS_NUMFILES > 0)
   int ix;
   struct romfs_fsdata_file *f;
   
   for (f = (struct romfs_fsdata_file *)ROMFS_ROOT;
        f != NULL;
        f = (struct romfs_fsdata_file *)f->next)
   {
      if (romfs_strcmp(path, (char *)f->name) == 0)
      {
         for (ix = 0; ix < CFG_ROMFS_MAXOPEN_FILES; ix++)
         {
            if (romfiles[ix].data == NULL)
            {
               romfiles[ix].fptr = 0;
               romfiles[ix].data = f->data;
               romfiles[ix].len = f->len;

               return ix;
            }
         }
         
         TRACE_ERROR("Not found empty file descriptor for file: %s", path);

         // Not found file descriptor
         return -1;
      }
   }
#endif
   return -1;
}


int romfs_close(int fd)
{
   if (fd < CFG_ROMFS_MAXOPEN_FILES)
   {
      romfiles[fd].data = NULL;
      return 0;
   }
   else
   {
      return -1;     
   }   
}

int romfs_read(int fd, void *buf, int count)
{
   uint8_t *pbuf = buf;
   uint16_t total = 0;
   romfs_file_t *file;
   
   if (fd < CFG_ROMFS_MAXOPEN_FILES && romfiles[fd].data != NULL)
      file = &romfiles[fd];
   else
      return -1;

   while(count && file->fptr < file->len)
   {
      *pbuf++ = file->data[file->fptr++];
      total++;
      count--;
   }

   return total;
}

/** Seek to offset */
off_t romfs_lseek(int fd, off_t offset, int whence)
{
   romfs_file_t *file;
   
   if (fd < CFG_ROMFS_MAXOPEN_FILES && romfiles[fd].data != NULL)
      file = &romfiles[fd];
   else
      return -1;
      
   switch(whence)
   {
      case SEEK_SET:
         if (offset > file->fptr)
            return -1;
         
         file->fptr = offset;
         break;
         
      case SEEK_CUR:
         if (file->fptr + offset > file->len)
            return -1;
         
         file->fptr += offset;
         break;
         
      case SEEK_END:
      default:
         return -1;
   }
   
   return file->fptr;
}

int romfs_fstat(int fd, struct stat *buf)
{
   romfs_file_t *file;
   
   if (fd < CFG_ROMFS_MAXOPEN_FILES && romfiles[fd].data != NULL)
      file = &romfiles[fd];
   else
      return -1;

   memset(buf, 0, sizeof(struct stat));
   buf->st_size = file->len;

   return 0;
}

/** Get romfs file by name */
struct romfs_fsdata_file *romfs_get_file(const char *path)
{
#if defined(ROMFS_NUMFILES) &&  (ROMFS_NUMFILES > 0)
   struct romfs_fsdata_file *f;
   
   for (f = (struct romfs_fsdata_file *)ROMFS_ROOT;
        f != NULL;
        f = (struct romfs_fsdata_file *)f->next)
   {
      if (romfs_strcmp(path, (char *)f->name) == 0)
      {
         return f;
      }
   }
#endif
   return NULL;
}

/** Get next romfs file */
struct romfs_fsdata_file *romfs_next_file(struct romfs_fsdata_file *curfile)
{
   struct romfs_fsdata_file *next;

   if (curfile == NULL)
   {
      next = (struct romfs_fsdata_file *)ROMFS_ROOT;
   }
   else
   {
      next = (struct romfs_fsdata_file *)curfile->next;
   }
   
   return next;
}

static inline uint8_t romfs_strcmp(const char *str1, const char *str2)
{
   uint8_t i;
   i = 0;
loop:

   if(str2[i] == 0 ||
         str1[i] == '\r' ||
         str1[i] == '\n') {
      return 0;
   }

   if(str1[i] != str2[i]) {
      return 1;
   }

   ++i;
   goto loop;
}


#if defined(CFG_ENABLE_ROMFS_POSIX) && (CFG_ENABLE_ROMFS_POSIX == 1)

int _open(const char *name, int flags, ...)
{
   TRACE("**** OPEN %s", name);
   return romfs_open(name, flags);
}

int _close(int fd)
{
    return romfs_close(fd);
}

ssize_t _read(int fd, void *buf, size_t count)
{
    return romfs_read(fd, buf, count);
}

ssize_t _write(int fd, const void *buf, size_t count)
{
    return -1;
}

off_t _lseek(int fd, off_t offset, int whence)
{
    return romfs_lseek(fd, offset, whence);
}

int _fstat(int fd, struct stat *buf)
{
    return romfs_fstat(fd, buf);
}

int _isatty(int fd)
{
    return 0;
}

#endif   // CFG_ENABLE_ROMFS_POSIX
   
#endif   // CFG_ENABLE_ROMFS