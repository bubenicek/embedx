
#include "voip_platform.h"

#define TRACE_TAG "voip-esp32"
#if !ENABLE_TRACE_VOIP
#undef TRACE
#define TRACE(...)
#endif


#define CFG_UDPSOCKET_THREAD_STACK_SIZE     4096
#define CFG_UDPSOCKET_THREAD_PRIORITY       (configMAX_PRIORITIES - 1)


// Prototypes:
static void udpsocket_thread(void *arg);

//
// Heap memory functions
//
/** Alloc memory */
void *voip_malloc(size_t size)
{
    return malloc(size);
}

/** Free memory */
void voip_free(void *ptr)
{
    free(ptr);
}

//
// Clock functions
//
voip_clock_time_t voip_clock_time(void)
{
    return hal_time_ms();
}


//
// IP address functions
//
/** Convert IP address to string */
const char *voip_ipaddr_ntoa(voip_ipaddr_t *ipaddr)
{
   return ipaddr_ntoa(&ipaddr->addr);
}

/** Convert string IP address */
int voip_ipaddr_aton(const char *ipaddr_str, voip_ipaddr_t *ipaddr)
{
   return ipaddr_aton(ipaddr_str, &ipaddr->addr) == 0 ? -1 : 0;
}

/** Set IP address */
void voip_ipaddr_set(voip_ipaddr_t *dst, voip_ipaddr_t *src)
{
   ip_addr_set(&dst->addr, &src->addr);
}


//
// UDP socket functions
//
/** Create UDP socket with defined receive packet callback and socket pool callback */
int voip_create_udp_socket(
         voip_udp_socket_t *socket,
         voip_ipaddr_t *ipaddr,
         uint16_t port,
         voip_recv_udp_callback_t recv_clbk,
         voip_poll_udp_callback_t poll_clbk,
         uint16_t poll_interval,
         void *user_param)
{
   err_t err;

   memset(socket, 0, sizeof(voip_udp_socket_t));
   socket->recv_clbk = recv_clbk;
   socket->poll_clbk = poll_clbk;
   socket->poll_interval = poll_interval;
   socket->user_param = user_param;
   voip_timer_set(&socket->poll_timer, poll_interval);

   // Set local IP address
   ip_addr_set(&ipaddr->addr, &netif_default->ip_addr);

   // Create new socket
   if ((socket->con = netconn_new(NETCONN_UDP)) == NULL)
   {
      TRACE_ERROR("Create TCP connection failed");
      goto fail_socket_new;
   }

   ip_set_option(socket->con->pcb.udp, SOF_REUSEADDR);

   // Bind socket to local port
   if ((err = netconn_bind(socket->con, IP_ADDR_ANY, port)) != ERR_OK)
   {
      TRACE_ERROR("Bind socket to port: %d failed, err: %d", port, err);
      goto fail_socket_bind;
   }

   // Set socket receive timeout
   netconn_set_recvtimeout(socket->con, poll_interval);

   // Create TX buffer
   if ((socket->txbuf = netbuf_new()) == NULL)
   {
      TRACE_ERROR("Alloc TX buffer");
      goto fail_alloctxbuf;
   }

   socket->state = UDP_SOCKET_OPENING;

   // Create task
   if (xTaskCreate(&udpsocket_thread, "udpsocket", CFG_UDPSOCKET_THREAD_STACK_SIZE, socket, CFG_UDPSOCKET_THREAD_PRIORITY, &socket->hthread) != pdPASS)
   {
      TRACE_ERROR("Create udpsocket thread failed");
      goto fail_thread;
   }

   TRACE("UDP socket %s:%d created", voip_ipaddr_ntoa(ipaddr), port);

   return 0;

fail_thread:
fail_alloctxbuf:
   netconn_close(socket->con);
fail_socket_bind:
   netconn_delete(socket->con);
fail_socket_new:
   socket->state = UDP_SOCKET_CLOSED;
   return -1;
}


/** Close and destroy UDP socket */
int voip_close_udp_socket(voip_udp_socket_t *socket)
{
   if (socket->con != NULL)
   {
      // close working thread
      socket->state = UDP_SOCKET_CLOSING;
      netconn_close(socket->con);

      // Wait for thread is destroyed
      while(socket->state == UDP_SOCKET_CLOSING)
         hal_delay_ms(10);

      // Free socket
      netbuf_delete(socket->txbuf);
      netconn_delete(socket->con);
      socket->con = NULL;

      TRACE("UDP socket closed");
   }

   return 0;
}

/** Send UDP packet */
int voip_send_udp_packet(voip_udp_socket_t *socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buffer, uint16_t buffer_length)
{
   err_t err;

#if defined(ENABLE_TRACE_SIP_RXTX) && (ENABLE_TRACE_SIP_RXTX == 1)
    if (port == SIP_PORT)
    {
        TRACE("*** Send %d bytes ***\n\n%s", buffer_length, (char*)buffer);
    }
#endif

   // reference the buffer
   netbuf_ref(socket->txbuf, buffer, buffer_length);

   // Send buffer
   err = netconn_sendto(socket->con, socket->txbuf, &ipaddr->addr, port);
   if (err != ERR_OK)
   {
      TRACE_ERROR("Send UDP packet to: %s failed, err: %d",  voip_ipaddr_ntoa(ipaddr), err);
      return -1;
   }

   return 0;
}

/** Resolve IP address */
int voip_dns_resolve_ipaddr(const char *hostname, voip_ipaddr_t *ipaddr)
{
   err_t err;

   err = netconn_gethostbyname(hostname, &ipaddr->addr);
   if (err != ERR_OK)
   {
      TRACE_ERROR("Gethostbyname %s failed", hostname);
      return -1;
   }

   return 0;
}

static void udpsocket_thread(void *arg)
{
   err_t res;
   struct netbuf *rxbuf;
   voip_ipaddr_t ipaddr;
   voip_udp_socket_t *socket = arg;

   TRACE("UDP Socket thread start");
   socket->state = UDP_SOCKET_OPEN;

   while(socket->state == UDP_SOCKET_OPEN)
   {
      res = netconn_recv(socket->con, &rxbuf);
      if (res == ERR_OK)
      {
         if (socket->recv_clbk != NULL)
         {
            ip_addr_set(&ipaddr.addr, netbuf_fromaddr(rxbuf));
            socket->recv_clbk(socket, &ipaddr, netbuf_fromport(rxbuf), rxbuf->p->payload, netbuf_len(rxbuf), socket->user_param);
         }

         netbuf_delete(rxbuf);
      }
      else if (res != ERR_TIMEOUT)
      {
         TRACE_ERROR("RX packet failed, res: %d", res);
      }

      if (voip_timer_expired(&socket->poll_timer))
      {
          if (socket->poll_clbk != NULL)
              socket->poll_clbk(socket, socket->user_param);

          voip_timer_reset(&socket->poll_timer);
      }
   }

   TRACE("UDP socket thread stop");

   socket->state = UDP_SOCKET_CLOSED;
   vTaskDelete(NULL);
}

