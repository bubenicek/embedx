
#include "system.h"
#include "voip_platform.h"


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
    static char ipaddr_txt[32];
/*
    snprintf(ipaddr_txt, sizeof(ipaddr_txt), "%u.%u.%u.%u",
             (unsigned char) ( ( GET_IPV4_ADDRESS(*ipaddr) >> 24 ) & 0xff ),
             (unsigned char) ( ( GET_IPV4_ADDRESS(*ipaddr) >> 16 ) & 0xff ),
             (unsigned char) ( ( GET_IPV4_ADDRESS(*ipaddr) >>  8 ) & 0xff ),
             (unsigned char) ( ( GET_IPV4_ADDRESS(*ipaddr) >>  0 ) & 0xff ));
*/
    return ipaddr_txt;
}

/** Convert string IP address */
int voip_ipaddr_aton(const char *ipaddr_str, voip_ipaddr_t *ipaddr)
{
/*
    ip_addr_t lwip_ipaddr;

    if (ipaddr_aton(ipaddr_str, &lwip_ipaddr) == 0)
    {
        return -1;
    }

    SET_IPV4_ADDRESS(*ipaddr, ntohl(lwip_ipaddr.addr));
*/
    return 0;
}



/** Set IP address */
void voip_ipaddr_set(voip_ipaddr_t *dst, voip_ipaddr_t *src)
{
//    SET_IPV4_ADDRESS(*dst, GET_IPV4_ADDRESS(*src));
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
    return 0;
}

/** Close and destroy UDP socket */
int voip_close_udp_socket(voip_udp_socket_t *socket)
{
    return 0;
}

/** Send UDP packet */
int voip_send_udp_packet(voip_udp_socket_t *udp_socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buffer, uint16_t buffer_length)
{
    return 0;
}

/** Resolve IP address */
int voip_dns_resolve_ipaddr(const char *hostname, voip_ipaddr_t *ipaddr)
{
    return 0;
}
