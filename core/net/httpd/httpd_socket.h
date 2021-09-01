
#ifndef __HTTPD_SOCKET_H
#define __HTTPD_SOCKET_H

//
// RAW socket API
//
int httpd_raw_socket_create(int port);
int httpd_raw_socket_connect(const char *host, int port);
int httpd_raw_socket_close(int sd);
int httpd_raw_socket_accept(int sd, struct sockaddr_in *remote_addr);
int httpd_raw_socket_send(int sd, const void *buf, int count);
int httpd_raw_socket_recv(int sd, void *buf, int bufsize);
int httpd_raw_socket_readto(int sd, char *buf, int bufsize, char c);

//
// SSL socket API
//
int httpd_ssl_socket_create(int port);
int httpd_ssl_socket_connect(const char *host, int port);
int httpd_ssl_socket_close(int sd);
int httpd_ssl_socket_accept(int sd, struct sockaddr_in *remote_addr);
int httpd_ssl_socket_send(int sd, const void *buf, int count);
int httpd_ssl_socket_recv(int sd, void *buf, int bufsize);
int httpd_ssl_socket_readto(int sd, char *buf, int bufsize, char c);


#if defined(CFG_HTTPD_USE_SSL_SOCKET) && (CFG_HTTPD_USE_SSL_SOCKET == 1)

#define httpd_socket_create   httpd_ssl_socket_create
#define httpd_socket_connect  httpd_ssl_socket_connect
#define httpd_socket_close    httpd_ssl_socket_close
#define httpd_socket_accept   httpd_ssl_socket_accept
#define httpd_socket_send     httpd_ssl_socket_send
#define httpd_socket_recv     httpd_ssl_socket_recv
#define httpd_socket_readto   httpd_ssl_socket_readto

#else

#define httpd_socket_create   httpd_raw_socket_create
#define httpd_socket_connect  httpd_raw_socket_connect
#define httpd_socket_close    httpd_raw_socket_close
#define httpd_socket_accept   httpd_raw_socket_accept
#define httpd_socket_send     httpd_raw_socket_send
#define httpd_socket_recv     httpd_raw_socket_recv
#define httpd_socket_readto   httpd_raw_socket_readto

#endif // CFG_HTTPD_USE_SSL_SOCKET


#endif // __HTTPD_SOCKET_H
