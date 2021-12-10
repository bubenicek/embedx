
#ifndef __HTTPCLIENT_H
#define __HTTPCLIENT_H

/** Connect host */
int httpclient_connect(const char *host, int port);

/** Send http GET request */
int httpclient_get(int sd, const char *url);

/** Read data from host */
int httpclient_read_data(int sd, uint8_t *buf, int bufsize);

/** Close connection with host */
int httpclient_close(int sd);

/** Get file from host and save it to local file */
int httpclient_get_file(const char *host, int port, const char *url, const char *filename);

#endif   // __HTTPCLIENT_H