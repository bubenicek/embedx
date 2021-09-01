
#include "httpd.h"
#include "httpd_fsdata.h"

// Include file system application data
#if defined (CFG_HTTPD_USE_FSDATA) && (CFG_HTTPD_USE_FSDATA == 1)
 #include "httpd/httpd_fsdata.c"
#endif

// Prototypes:
static inline uint8_t httpd_fs_strcmp(const char *str1, const char *str2);


/** Open file */
int httpd_file_open(httpd_file_t *file, const char *name)
{
#if defined(HTTPD_FS_NUMFILES) &&  (HTTPD_FS_NUMFILES > 0)
   struct httpd_fsdata_file *f;

   for (f = (struct httpd_fsdata_file *)HTTPD_FS_ROOT;
        f != NULL;
        f = (struct httpd_fsdata_file *)f->next)
   {
      if(httpd_fs_strcmp(name, (char *)f->name) == 0)
      {
         file->fptr = 0;
         file->data = f->data;
         file->len = f->len;
         return 0;
      }
   }
#endif
   return -1;
}

int httpd_file_close(httpd_file_t *file)
{
   return 0;
}

int httpd_file_read(httpd_file_t *file, void *buf, int count)
{
   uint8_t *pbuf = buf;
   uint16_t total = 0;

   while(count && file->fptr < file->len)
   {
      *pbuf++ = file->data[file->fptr++];
      total++;
      count--;
   }

   return total;
}

/** Read line from file */
int httpd_file_getline(httpd_file_t *file, char *buf, int bufsz)
{
   int res;
   int total = 0;

   while(bufsz > 0)
   {
      res = httpd_file_read(file, buf, 1);
      if (!res)
         break;      // EOF
      if (res < 0)
         return res; // error

      bufsz--;
      total++;
      if (*buf++ == '\n')
         break;
   }

   *buf = 0;

   return total;
}


static inline uint8_t httpd_fs_strcmp(const char *str1, const char *str2)
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
