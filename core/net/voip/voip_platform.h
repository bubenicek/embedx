
#ifndef __voip_platform_h
#define __voip_platform_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "system.h"

#include "voip_config.h"
#include "voip_timer.h"

//
// Heap memory functions
//

/** Alloc memory */
void *voip_malloc(size_t size);

/** Free memory */
void voip_free(void *ptr);


//
// Clock functions
//
voip_clock_time_t voip_clock_time(void);


//
// IP address functions
//

struct voip_ipaddr;

/** Convert IP address to string */
const char *voip_ipaddr_ntoa(struct voip_ipaddr *ipaddr);

/** Convert string IP address */
int voip_ipaddr_aton(const char *ipaddr_str, struct voip_ipaddr *ipaddr);

/** Set IP address */
void voip_ipaddr_set(struct voip_ipaddr *dst, struct voip_ipaddr *src);


//
// UDP socket functions
//

struct voip_udp_socket;

/** Receive UDP packet callback */
typedef void (*voip_recv_udp_callback_t)(struct voip_udp_socket *socket, struct voip_ipaddr *ipaddr, uint16_t port, void *buf, uint16_t buflen, void *user_param);

/** Poll UDP socket in defined interval */
typedef void (*voip_poll_udp_callback_t)(struct voip_udp_socket *socket, void *user_param);


/** Create UDP socket */
int voip_create_udp_socket(
         struct voip_udp_socket *socket,
         struct voip_ipaddr *ipaddr,
         uint16_t port,
         voip_recv_udp_callback_t recv_packet_clbk,
         voip_poll_udp_callback_t poll_clbk,
         uint16_t poll_interval,
         void *user_param);

/** Close and destroy UDP socket */
int voip_close_udp_socket(struct voip_udp_socket *socket);

/** Send UDP packet */
int voip_send_udp_packet(struct voip_udp_socket *socket, struct voip_ipaddr *ipaddr, uint16_t port, void *buf, uint16_t buflen);

/** Resolve IP address */
int voip_dns_resolve_ipaddr(const char *hostaddr, struct voip_ipaddr *ipaddr);


#if defined(VOIP_PLATFORM_WICED) && (VOIP_PLATFORM_WICED == 1)
#include "platform/wiced/voip_platform_wiced.h"
#elif defined (VOIP_PLATFORM_ESP32) && (VOIP_PLATFORM_ESP32 == 1)
#include "platform/esp32/voip_platform_esp32.h"
#elif defined (VOIP_PLATFORM_LINUX) && (VOIP_PLATFORM_LINUX == 1)
#include "platform/linux/voip_linux_platform.h"
#else
#error "Unknown VOIP platform"
#endif


#endif // voip_platform.h
