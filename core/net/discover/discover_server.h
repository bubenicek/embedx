
#ifndef __DISCOVER_SERVER_H
#define __DISCOVER_SERVER_H

#ifndef CFG_DISCOVER_PORT
#define CFG_DISCOVER_PORT                   12344
#endif

#ifndef CFG_DISCOVER_BUFFER_SIZE
#define CFG_DISCOVER_BUFFER_SIZE            512
#endif

#ifndef CFG_DISCOVER_THREAD_PRIORITY
#define CFG_DISCOVER_THREAD_PRIORITY        osPriorityNormal
#endif

#ifndef CFG_DISCOVER_THREAD_STACK_SIZE
#define CFG_DISCOVER_THREAD_STACK_SIZE      1024
#endif


typedef int (*discover_server_request_cb_t)(char *buf, int bufsize);

/** Initialize discover client */
int discover_server_init(const discover_server_request_cb_t request_cb);

#endif // __DISCOVER_SERVER_H
