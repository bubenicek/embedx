#include <stdint.h>
#include <stdbool.h>
#include "voip_platform.h"

#define DEBUG_TAG  "voip_wiced"

#define DNS_RESOLVE_TIMEOUT               10000
#define UDP_SOCKET_THREAD_STACK_SIZE      4096

// Prototypes:
static void udp_socket_thread(uint32_t arg);

/** UDP socket extension data */
typedef struct
{
    wiced_thread_t socket_thread;
    bool terminate_thread;
    voip_timer_t poll_timer;

    voip_recv_udp_callback_t recv_clbk;
    voip_poll_udp_callback_t poll_clbk;
    uint16_t poll_interval;
    void *user_param;

} voip_udp_socket_ext_t;


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
    return host_rtos_get_time();
}


//
// IP address functions
//

/** Convert IP address to string */
const char *voip_ipaddr_ntoa(voip_ipaddr_t *ipaddr)
{
    static char ipaddr_txt[32];

    snprintf(ipaddr_txt, sizeof(ipaddr_txt), "%u.%u.%u.%u",
             (unsigned char) ( ( GET_IPV4_ADDRESS(*ipaddr) >> 24 ) & 0xff ),
             (unsigned char) ( ( GET_IPV4_ADDRESS(*ipaddr) >> 16 ) & 0xff ),
             (unsigned char) ( ( GET_IPV4_ADDRESS(*ipaddr) >>  8 ) & 0xff ),
             (unsigned char) ( ( GET_IPV4_ADDRESS(*ipaddr) >>  0 ) & 0xff ));

    return ipaddr_txt;
}

/** Convert string IP address */
int voip_ipaddr_aton(const char *ipaddr_str, voip_ipaddr_t *ipaddr)
{
    ip_addr_t lwip_ipaddr;

    if (ipaddr_aton(ipaddr_str, &lwip_ipaddr) == 0)
    {
        return -1;
    }

    SET_IPV4_ADDRESS(*ipaddr, ntohl(lwip_ipaddr.addr));

    return 0;
}



/** Set IP address */
void voip_ipaddr_set(voip_ipaddr_t *dst, voip_ipaddr_t *src)
{
    SET_IPV4_ADDRESS(*dst, GET_IPV4_ADDRESS(*src));
}


//
// UDP socket functions
//

/** Create UDP socket with defined receive packet callback and socket pool callback */
int voip_create_udp_socket(voip_udp_socket_t *socket, voip_ipaddr_t *ipaddr, uint16_t port,
                           voip_recv_udp_callback_t recv_clbk, voip_poll_udp_callback_t poll_clbk,
                           uint16_t poll_interval, void *user_param)
{
    voip_udp_socket_ext_t *ext;

    if (wiced_ip_get_ipv4_address(WICED_STA_INTERFACE, ipaddr) != WICED_SUCCESS)
    {
        error("Get local interface IP address");
        goto fail_get_ipaddr;
    }

    if (wiced_udp_create_socket(socket, port, WICED_STA_INTERFACE ) != WICED_SUCCESS)
    {
        voip_error("UDP socket creation failed");
        goto fail_create_socket;
    }

    ext = malloc(sizeof(voip_udp_socket_ext_t));
    if (ext == NULL)
    {
        voip_error("alloc udp socket ext");
        goto fail_alloc;
    }

    memset(ext, 0, sizeof(voip_udp_socket_ext_t));
    ext->recv_clbk = recv_clbk;
    ext->poll_clbk = poll_clbk;
    ext->poll_interval = poll_interval;
    ext->user_param = user_param;
    voip_timer_set(&ext->poll_timer, poll_interval);

    socket->user_data = ext;

    // create working thread for receiving and pooling
    if (wiced_rtos_create_thread(&ext->socket_thread, WICED_DEFAULT_WORKER_PRIORITY, NULL, udp_socket_thread, UDP_SOCKET_THREAD_STACK_SIZE, socket) != WICED_SUCCESS)
    {
        error("Create udp socket thread");
        goto fail_create_thread;
    }

    debug("UDP socket %s:%d created", voip_ipaddr_ntoa(ipaddr), port);

    return 0;

fail_create_thread:
fail_alloc:
    wiced_udp_delete_socket(socket);
fail_create_socket:
fail_get_ipaddr:
    return -1;
}

/** Close and destroy UDP socket */
int voip_close_udp_socket(voip_udp_socket_t *socket)
{
    voip_udp_socket_ext_t *ext;

    if (socket->user_data == NULL)
    {
        error("UDP socket is already closed");
        return -1;
    }

    ext = socket->user_data;

    // close working thread
    ext->terminate_thread = true;
    wiced_rtos_thread_join(&ext->socket_thread);
    wiced_rtos_delete_thread(&ext->socket_thread);

    free(socket->user_data);
    socket->user_data = NULL;

    if (wiced_udp_delete_socket(socket) != WICED_SUCCESS)
    {
        error("Close UDP socket");
        return -1;
    }

    debug("Socket closed");

    return 0;
}

/** Send UDP packet */
int voip_send_udp_packet(voip_udp_socket_t *udp_socket, voip_ipaddr_t *ipaddr, uint16_t port, void *buffer, uint16_t buffer_length)
{
    wiced_packet_t*          packet;
    char*                    tx_data;
    uint16_t                 available_data_length;

    // Create the UDP packet. Memory for the TX data is automatically allocated
    if (wiced_packet_create_udp(udp_socket, buffer_length, &packet, (uint8_t**) &tx_data, &available_data_length) != WICED_SUCCESS)
    {
        voip_error("UDP tx packet creation failed");
        return -1;
    }

    // Copy buffer into tx_data which is located inside the UDP packet
    memcpy(tx_data, buffer, buffer_length);

    // Set the end of the data portion of the packet
    wiced_packet_set_data_end(packet, (uint8_t *)tx_data + buffer_length);

    // Send the UDP packet
    if (wiced_udp_send(udp_socket, ipaddr, port, packet) != WICED_SUCCESS)
    {
        error("UDP packet send failed");
        wiced_packet_delete(packet);
        return -1;
    }

    /*
     * NOTE : It is not necessary to delete the packet created above, the packet
     *        will be automatically deleted *AFTER* it has been successfully sent
     */
     
    return 0;
}

/** Resolve IP address */
int voip_dns_resolve_ipaddr(const char *hostname, voip_ipaddr_t *ipaddr)
{
    if (wiced_hostname_lookup(hostname, ipaddr, DNS_RESOLVE_TIMEOUT) != WICED_SUCCESS)
    {
        error("Resolve hostname: %s", hostname);
        return -1;
    }

    return 0;
}

/** Thread loop for receiving packet and polling UDP socket connection */
static void udp_socket_thread(uint32_t arg)
{
    wiced_packet_t            *packet;
    uint8_t                   *rx_data;
    static uint16_t           rx_data_length;
    uint16_t                  available_data_length;
    wiced_ip_address_t        udp_src_ip_addr;
    uint16_t                  udp_src_port;
    wiced_result_t            result;
    wiced_udp_socket_t        *udp_socket = (wiced_udp_socket_t *)arg;
    voip_udp_socket_ext_t     *ext = udp_socket->user_data;

    debug("Socket thread start");
    
    while(!ext->terminate_thread)
    {
        // Wait for UDP packet
        result = wiced_udp_receive(udp_socket, &packet, ext->poll_interval);
        if (result == WICED_SUCCESS)
        {
            // Get info about the received UDP packet
            if (wiced_udp_packet_get_info(packet, &udp_src_ip_addr, &udp_src_port) == WICED_SUCCESS)
            {
               // Extract the received data from the UDP packet
               if (wiced_packet_get_data(packet, 0, (uint8_t**) &rx_data, &rx_data_length, &available_data_length) == WICED_SUCCESS)
               {
                  // Invoke recv callback
                  if (ext->recv_clbk != NULL)
                  {
                     //debug("RX %d", rx_data_length);
                     ext->recv_clbk(udp_socket, &udp_src_ip_addr, udp_src_port, rx_data, rx_data_length, ext->user_param);
                  }
               }
               else
               {
                  error("wiced_packet_get_data failed");
               }
            }
            else
            {
               error("wiced_udp_packet_get_info failed");
            }

            // Delete the received packet, it is no longer needed */
            wiced_packet_delete(packet);

            if (voip_timer_expired(&ext->poll_timer))
            {
                // Run pool function
                if (ext->poll_clbk != NULL)
                {
                    ext->poll_clbk(udp_socket, ext->user_param);
                }
                voip_timer_reset(&ext->poll_timer);
            }
        }
        else if (result == WICED_TIMEOUT)
        {
            // Run pool function
            if (ext->poll_clbk != NULL)
            {
                ext->poll_clbk(udp_socket, ext->user_param);
            }
            voip_timer_reset(&ext->poll_timer);
        }
        else
        {
            error("receive packet result: %d", result);
        }
    }

    debug("Socket thread stop");

    // Relese thread
    wiced_rtos_delete_thread(&ext->socket_thread);
}
