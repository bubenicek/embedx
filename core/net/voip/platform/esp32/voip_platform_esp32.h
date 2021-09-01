
#ifndef __voip_platform_esp32_h
#define __voip_platform_esp32_h

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/ip4.h"
#include "lwip/ip4_addr.h"

#include "voip_platform.h"


/** IP address type */
typedef struct voip_ipaddr
{
   ip_addr_t addr;

} voip_ipaddr_t;


typedef enum
{
    UDP_SOCKET_OPENING,
    UDP_SOCKET_OPEN,
    UDP_SOCKET_CLOSING,
    UDP_SOCKET_CLOSED

} voip_udp_socket_state_t;



/** UDP socket */
typedef struct voip_udp_socket
{
    TaskHandle_t hthread;
    voip_udp_socket_state_t state;
    voip_timer_t poll_timer;

    struct netconn *con;
    struct netbuf *txbuf;

    voip_recv_udp_callback_t recv_clbk;
    voip_poll_udp_callback_t poll_clbk;
    uint16_t poll_interval;
    void *user_param;

} voip_udp_socket_t;


#endif
