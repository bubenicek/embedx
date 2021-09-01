//
// POSIX file weak functions template
//

#include "system.h"

struct stat;

int _open(const char *name, int flags, ...)
{
   return -1;
}

int _close(int fd)
{
    return -1;
}

ssize_t _read(int fd, void *buf, size_t count)
{
    return -1;
}

ssize_t _write(int fd, const void *buf, size_t count)
{
    return -1;
}

off_t _lseek(int fd, off_t offset, int whence)
{
    return -1;
}

int _fstat(int fd, struct stat *buf)
{
    return -1;
}

int _stat(const char *pathname, struct stat *statbuf)
{
   return -1;
}

int _isatty(int fd)
{
    return -1;
}

int _unlink(const char *pathname)
{
   return -1;
}

int creat(const char *pathname, mode_t mode)
{
   return -1;
}

int mkdir(const char *__path, __mode_t __mode)
{
   return -1;
}

int rmdir (const char *__path)
{
   return -1;
}

