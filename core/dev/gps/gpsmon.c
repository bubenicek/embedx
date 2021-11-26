
#include "system.h"
#include "minmea.h"
#include "gps.h"
#include "gps_driver.h"

TRACE_TAG(gpsmon);
#if !ENABLE_TRACE_GPS
#undef TRACE
#define TRACE(...)
#endif


#ifndef CFG_GPSMON_PORT
#define CFG_GPSMON_PORT     2021
#endif

#ifndef CFG_GPSMON_THREAD_STACK_SIZE
#define CFG_GPSMON_THREAD_STACK_SIZE        4096
#endif

#ifndef CFG_GPSMON_THREAD_PRIORITY
#define CFG_GPSMON_THREAD_PRIORITY          osPriorityNormal
#endif


// Prototypes:
static void gpsmon_thread(void *arg);

// Locals:
static const osThreadDef(GPSMON, gpsmon_thread, CFG_GPSMON_THREAD_PRIORITY, 0, CFG_GPSMON_THREAD_STACK_SIZE);
static int server_socket;
static int client_socket;
static bool connected = false;


/** Initialize GPS socket monitor */
int gpsmon_init(void)
{
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) 
    {
        TRACE_ERROR("Create socket failed");
        return -1;
    }
        
    int yes = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0 ) 
    {
        TRACE_ERROR("setsockopt failed");
        return -1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(CFG_GPSMON_PORT);

    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        TRACE_ERROR("Bind socket failed");
        return -1;
    }

    if (listen(server_socket, 5) < 0)
    {
        TRACE_ERROR("Listen socket failed");
        return -1;
    }

    // Start monitor thread
    if (osThreadCreate(osThread(GPSMON), NULL) == 0)
    {
        TRACE_ERROR("Start GPSMON thread failed");
        return -1;
    }

    TRACE("Init");

    return 0;
}

/** Send buffer to open socket if it exists*/
int gpsmon_send(uint8_t *buf, int bufsize)
{
    int res = 0;

    if (connected)
    {
        res = send(client_socket, buf, bufsize, 0);
    }

    return res;
}

static void gpsmon_thread(void *arg)
{
    int res;
    struct sockaddr_in client_address;
    socklen_t address_size = sizeof(client_address);
    uint8_t buf[255];

    TRACE("Running ...");

    while(1)
    {
        TRACE("Waiting for connection ...");

        if ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, &address_size)) < 0)
        {
            TRACE_ERROR("Accept connection failed");
            continue;
        }

        TRACE("New connection accepted");
        connected = true;

        while(1)
        {
            // Reading data from a TCP client 
            if ((res = recv(client_socket, buf, sizeof(buf), 0)) > 0)
            {
                gps_driver_write(buf, res);
                TRACE("Write GPS data");
                TRACE_DUMP(buf, res);
            }
            else  if (res == 0) 
            {
                TRACE("Client disconnected");
                break;
            }
            else 
            {
                TRACE_ERROR("Read from socket failed");
            }
        }

        connected = false;
    }
}
