

#ifndef __HTTPD_FILE_H
#define __HTTPD_FILE_H


typedef struct
{
   const uint8_t *data;
   int len;
	uint32_t	fptr;			   //! File R/W pointer

} httpd_file_t;


int httpd_file_open(httpd_file_t *file, const char *name);
int httpd_file_close(httpd_file_t *file);
int httpd_file_read(httpd_file_t *file, void *buf, int count);
int httpd_file_getline(httpd_file_t *file, char *buf, int bufsz);

#endif // __HTTPD_FILE_H
