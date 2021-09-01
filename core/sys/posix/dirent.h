
#ifndef __DIRENT_H
#define __DIRENT_H

typedef struct
{

} DIR;

struct dirent
{
   unsigned short int d_reclen;
   unsigned char d_type;
   char d_name[256];
};

DIR *opendir (const char *name);
struct dirent *readdir (DIR *dirp);
int closedir (DIR *dirp);

#endif   // __DIRENT_H
