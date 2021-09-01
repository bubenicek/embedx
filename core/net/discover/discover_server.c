/**
 * \file discover_server.c       \brief UDP discover server
 */

#include "system.h"

#include "lwip/api.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"
#include "lwip/udp.h"

#include "discover_server.h"

#define ENABLE_TRACE_DISCOVER  1

TRACE_TAG(discover);
#if !ENABLE_TRACE_DISCOVER
#undef TRACE
#define TRACE(...)
#endif


// Prototypes:
static int send_response(struct netconn *con, ip_addr_t *addr, uint16_t port, char *data, int datalen);
static void discover_thread(void *arg);

// Locals:
static const osThreadDef(DISCOVER, discover_thread, CFG_DISCOVER_THREAD_PRIORITY, 0, CFG_DISCOVER_THREAD_STACK_SIZE);
static discover_server_request_cb_t request_cb;
static struct netconn *udpcon;
static char buffer[CFG_DISCOVER_BUFFER_SIZE];


/** Initialize discover client */
int discover_server_init(const discover_server_request_cb_t _request_cb)
{
    ip4_addr_t ipgroup;

    IP4_ADDR(&ipgroup, 255, 255, 255, 255);
    request_cb = _request_cb;

    // Join to MDNS multicats group
    netif_default->flags |= NETIF_FLAG_IGMP;
    igmp_start(netif_default);
    igmp_joingroup((ip4_addr_t *)IP_ADDR_ANY, &ipgroup);

    // Alloc UDP connection
    if ((udpcon = netconn_new(NETCONN_UDP)) == NULL)
    {
        TRACE_ERROR("Alloc connection");
        goto fail_create_con;
    }

    if (netconn_bind(udpcon, IP_ADDR_ANY, CFG_DISCOVER_PORT) != ERR_OK)
    {
        TRACE_ERROR("Bind connection");
        goto fail_bind_con;
    }

    if (osThreadCreate(osThread(DISCOVER), NULL) == NULL)
    {
      TRACE_ERROR("Start thread");
      throw_exception(fail_thread);
    }

    TRACE("Discover server initialized, listening on UDP port: %d", CFG_DISCOVER_PORT);

    return 0;

fail_thread:
fail_bind_con:
    netconn_delete(udpcon);
fail_create_con:
    return -1;
}

static int send_response(struct netconn *con, ip_addr_t *addr, uint16_t port, char *data, int datalen)
{
    struct netbuf *buf;

    // Alloc new buffer
    if ((buf = netbuf_new()) == NULL)
    {
        TRACE_ERROR("Alloc netbuf");
        return -1;
    }

    netbuf_ref(buf, data, datalen);
    netconn_sendto(con, buf, addr, port);

    netbuf_delete(buf);

    return 0;
}

static void discover_thread(void *arg)
{
    int res;
    uint16_t len;
    char *rxdata;
    struct netbuf *rxbuf;

    TRACE("Thread started");

    while(1)
    {
        if (netconn_recv(udpcon, &rxbuf) == ERR_OK)
        {
            if (netbuf_data(rxbuf, (void **)&rxdata, &len) == ERR_OK)
            {
                rxdata[len] = '\0';

                if (strncasecmp(rxdata, "hello", 5) == 0)
                {
                    res = request_cb(buffer, sizeof(buffer));
                    if (res > 0)
                    {
                        send_response(udpcon, &rxbuf->addr, rxbuf->port, buffer, res);
                    }
                }
                else
                {
                    TRACE_ERROR("Not supported request %s", rxdata);
                }
            }
            else
            {
                TRACE_ERROR("Read packet data");
            }

            netbuf_delete(rxbuf);
        }
        else
        {
            TRACE_ERROR("Recv packet");
        }
    }
}
