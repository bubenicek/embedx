/**
 * \file httpd.h   \brief Embedded HTTP server
 */

#ifndef __HTTPD_H
#define __HTTPD_H

#include "system.h"

#if defined(CFG_USE_LWIP) && (CFG_USE_LWIP == 1)

#include <lwip/netif.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <lwip/netdb.h>
#include <lwip/sys.h>
#include <lwip/mem.h>
#include <lwip/netif.h>
#include <lwip/sockets.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#else

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#endif   // CFG_USE_LWIP

#include "httpd_socket.h"
#include "httpd_file.h"
#include "httpd_cgi.h"
#include "httpd_cookie.h"
#include "httpd_rest.h"


//
// Default configuration
//

// Max number of active connections
#ifndef CFG_HTTPD_MAXNUM_CONNECTIONS
#define CFG_HTTPD_MAXNUM_CONNECTIONS                1       
#endif

#ifndef CFG_HTTPD_THREAD_STACK_SIZE
#define CFG_HTTPD_THREAD_STACK_SIZE                 2048
#endif

#ifndef CFG_HTTPD_LISTENER_THREAD_STACK_SIZE
#define CFG_HTTPD_LISTENER_THREAD_STACK_SIZE        1024
#endif

#ifndef CFG_HTTPD_THREAD_PRIORITY
#define CFG_HTTPD_THREAD_PRIORITY                  osPriorityNormal
#endif

#ifndef CFG_HTTPD_USE_BASIC_AUTHORIZATION
#define CFG_HTTPD_USE_BASIC_AUTHORIZATION         0
#endif

#ifndef CFG_HTTPD_USE_RESTAPI_AUTHORIZATION
#define CFG_HTTPD_USE_RESTAPI_AUTHORIZATION       0
#endif

#ifndef CFG_HTTPD_READ_SOCKET_TIMEOUT
#define CFG_HTTPD_READ_SOCKET_TIMEOUT            5000
#endif

#ifndef CFG_HTTPD_LISTEN_BACKLOG
#define CFG_HTTPD_LISTEN_BACKLOG                 4
#endif

#ifndef CFG_HTTPD_MAX_POST_WWFORM_SIZE
#define CFG_HTTPD_MAX_POST_WWFORM_SIZE  2048
#endif

#ifndef CFG_HTTPD_REST_MAX_PARAMS
#define CFG_HTTPD_REST_MAX_PARAMS      8
#endif

#ifndef CFG_HTTPD_WWW_ROOT_DIR
#define CFG_HTTPD_WWW_ROOT_DIR         "www"
#endif

#ifndef CFG_HTTPD_USE_SSL_SOCKET
#define CFG_HTTPD_USE_SSL_SOCKET       0
#endif

#define CFG_HTTPD_RAW_PORT             80
#define CFG_HTTPD_SSL_PORT             443

#ifndef CFG_HTTPD_GZIP_FILE_CONTENT
#define CFG_HTTPD_GZIP_FILE_CONTENT    0
#endif

#ifndef CFG_HTTPD_USE_FSDATA
#define CFG_HTTPD_USE_FSDATA           0
#endif


//
// Trace options
//
#ifndef ENABLE_TRACE_HTTPD
#define ENABLE_TRACE_HTTPD             0
#endif
#ifndef ENABLE_TRACE_HTTPD_CGI
#define ENABLE_TRACE_HTTPD_CGI         0
#endif
#ifndef ENABLE_TRACE_HTTPD_SOCKET
#define ENABLE_TRACE_HTTPD_SOCKET      0
#endif

#define HTTPD_FILENAME_LENGTH          128        // Input URI length
#define HTTPD_AUTH_LENGTH              64         // Authentication string length
#define HTTPD_BUFFER_SIZE              1024       // Input/output buffer size
#define HTTPD_LINE_SIZE                255
#define HTTPD_COOKIES_LENGTH           255        // Cookies buffer length

/** Request method types */
typedef enum
{
   HTTPD_REQ_TYPE_GET,
   HTTPD_REQ_TYPE_POST,
   HTTPD_REQ_TYPE_PUT,
   HTTPD_REQ_TYPE_DELETE

} httpd_req_type_e;


/** Request content types */
typedef enum
{
   HTTPD_CONTENT_TYPE_PLAIN,
   HTTPD_CONTENT_TYPE_WWWFORM_ENCODED,

} httpd_content_type_e;


#define ISO_NL             0x0a
#define ISO_SPACE          0x20
#define ISO_BANG           0x21
#define ISO_PERCENT        0x25
#define ISO_PERIOD         0x2e
#define ISO_SLASH          0x2f
#define ISO_COLON          0x3a

#define HTTP_HTTP          "http://"
#define HTTP_200           "200 "
#define HTTP_301           "301 "
#define HTTP_302           "302 "
#define HTTP_GET           "GET "
#define HTTP_POST          "POST "
#define HTTP_PUT           "PUT "
#define HTTP_DELETE        "DELETE "
#define HTTP_10            "HTTP/1.0"
#define HTTP_11            "HTTP/1.1"
#define HTTP_CONTENT_TYPE  "content-type: "
#define HTTP_TEXTHTML      "text/html"
#define HTTP_LOCATION      "location: "
#define HTTP_HOST          "host: "
#define HTTP_CRNL          "\r\n"
#define HTTP_INDEX_HTML    "index.html"
#define HTTP_404_HTML      "/404.html"
#define HTTP_HTML          ".html"
#define HTTP_SHTML         ".stm"
#define HTTP_HTM           ".html"
#define HTTP_CSS           ".css"
#define HTTP_PNG           ".png"
#define HTTP_GIF           ".gif"
#define HTTP_JPG           ".jpg"
#define HTTP_JSON          ".json"
#define HTTP_TEXT          ".txt"
#define HTTP_TXT           ".txt"
#define HTTP_REFERER       "Referer:"
#define HTTP_HEADER_200    "HTTP/1.1 200 OK\r\nServer: httpd/1.1\r\nConnection: close\r\n"
#define HTTP_HEADER_200_GZIP "HTTP/1.1 200 OK\r\nServer: httpd/1.0\r\nConnection: close\r\nContent-Encoding: gzip\r\n"
#define HTTP_HEADER_201    "HTTP/1.1 201 Created\r\nServer: httpd/1.0\r\nConnection: close\r\n"
#define HTTP_HEADER_301    "HTTP/1.1 301 Moved Permanently\r\n"
#define HTTP_HEADER_400    "HTTP/1.1 400 Bad Request\r\nServer: httpd/1.0\r\nConnection: close\r\n"
#define HTTP_HEADER_401_BASIC "HTTP/1.1 401 Please Authenticate\r\nServer: httpd/1.0\r\nWWW-Authenticate: Basic realm=\"2N ZW Controller\"\r\nConnection: close\r\n"
#define HTTP_HEADER_401    "HTTP/1.1 401 Please Authenticate\r\nServer: httpd/1.0\r\nConnection: close\r\n"
#define HTTP_HEADER_404    "HTTP/1.1 404 Not found\r\nServer: httpd/1.0\r\nConnection: close\r\n"
#define HTTP_HEADER_405    "HTTP/1.1 405 Method Not Allowed\r\nServer: httpd/1.0\r\nConnection: close\r\n"
#define HTTP_HEADER_406    "HTTP/1.1 406 Not Acceptable\r\nServer: httpd/1.0\r\nConnection: close\r\n"
#define HTTP_HEADER_500    "HTTP/1.1 500 Internal Server Error\r\nServer: httpd/1.0\r\nConnection: close\r\n"
#define HTTP_BODY_404       "<html><center><h1>404 - file not found</h1><h3>Go <a href='/'>here</a> instead.</h3></center></body></html>"
#define HTTP_CONTENT_TYPE_PLAIN     "Content-type: text/plain\r\n\r\n"
#define HTTP_CONTENT_TYPE_HTML      "Content-type: text/html\r\n\r\n"
#define HTTP_CONTENT_TYPE_CSS       "Content-type: text/css\r\n\r\n"
#define HTTP_CONTENT_TYPE_TEXT      "Content-type: text/text\r\n\r\n"
#define HTTP_CONTENT_TYPE_PNG       "Content-type: image/png\r\n\r\n"
#define HTTP_CONTENT_TYPE_GIF       "Content-type: image/gif\r\n\r\n"
#define HTTP_CONTENT_TYPE_JPG       "Content-type: image/jpeg\r\n\r\n"
#define HTTP_CONTENT_TYPE_BINARY    "Content-type: application/octet-stream\r\n\r\n"
#define HTTP_CONTENT_TYPE_WWW_FORM  "Content-Type: application/x-www-form-urlencoded"
#define HTTP_CONTENT_TYPE_JSON      "Content-type: application/json\r\n\r\n"
#define HTTP_CONTENT_LENGTH         "Content-Length: "
#define HTTP_CONTENT_DISPOSITION_FORMDATA  "Content-Disposition: form-data;"
#define HTTP_CONTENT_TYPE_DISPOSITION_FILENAME  "Content-Disposition: filename=\"%s\"\r\n"
#define HTTP_AUTHORIZATION_BASIC    "Authorization: Basic "



/** HTTP connection */
typedef struct httpd_connection
{
   struct httpd *httpd;                         // Parent http server
   osThreadId thread;                           // Thread ID
   hal_time_t start_tm;                         // Start request time

   int sd;                                      // Socket descriptor
   struct sockaddr_in remote_addr;              // Remote IP address

   httpd_req_type_e reqtype;                    // Request type (POST || GET)
   httpd_content_type_e content_type;           // Type of POST data WWWFORM || FILE
   int content_length;                          // Content length

   char buffer[HTTPD_BUFFER_SIZE];              // Input/output buffer
   char filename[HTTPD_FILENAME_LENGTH];        // Requested filename
   char authorization[HTTPD_AUTH_LENGTH];       // Authorization string
   char cookies[HTTPD_COOKIES_LENGTH];          // Cookies string
   int output_cookies_length;                   // Length of cookies to output

   char *input_data;                            // Ptr to allocated input request data
   uint8_t header_send;                         // HTTP header was send

   uint8_t keep_connection;                     // Keep connection flag

   uint8_t longpolling;                         // Long pooling for requests

#if defined (HTTPD_CON_REST_API_CONTEXT)
   HTTPD_CON_REST_API_CONTEXT;
#endif

} httpd_connection_t;


/** HTTP server */
typedef struct httpd
{
   /** Listening port number */
   uint16_t port;

   /** Listner socket descriptor */
   int sd;

   /** Thread ID */
   osThreadId thread;

   /** Max connections semaphore */
   osSemaphoreId consem;

} httpd_t;


/** Initialize HTTP server */
int httpd_init(httpd_t *s, uint16_t port);

/** Deinitialize HTTP server */
int httpd_deinit(httpd_t *s);

/** Send buffer */
#define httpd_send(_con, _buf, _count) \
   httpd_socket_send((_con)->sd, _buf, _count)

/** Receive data  */
#define httpd_recv(_con, _buf, _bufsize) \
   httpd_socket_recv((_con)->sd, _buf, _bufsize)

/** Read from socket until required char is received */
#define httpd_readto(_con, _buf, _bufsize, _c) \
   httpd_socket_readto((_con)->sd, _buf, _bufsize, _c)

#define httpd_set_content_filename(_con, _filename) \
   strlcpy((_con)->filename, _filename, sizeof((_con)->filename))

/** Send HTTP header */
void httpd_send_headers(httpd_connection_t *s, const char *statushdr);

/** Receive data to file */
int httpd_recv_file(httpd_connection_t *s, const char *filename);

/** Send data from file */
int httpd_send_file(httpd_connection_t *s, const char *filename);


/**
 * funkce ktera provadi overeni uzivatele a hesla, musi byt implementovana v aplikaci
 * pokud je vyzadovana BASIC authorizace web pristupu
 * @param user uzivatelske jmeno
 * @param passwd heslo
 * @return 1 if success authorized, 0 if not authorized
 */
extern uint8_t httpd_basic_authorize(char *user, char *passwd);


#endif   // __HTTPD_H
