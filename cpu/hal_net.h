
#ifndef __HAL_NET_H
#define __HAL_NET_H


#if BYTE_ORDER == BIG_ENDIAN
/** Set an IP address given by the four byte-parts */
#define HAL_NET_IPADDR(a,b,c,d) \
         ((uint32_t)((a) & 0xff) << 24) | \
          ((uint32_t)((b) & 0xff) << 16) | \
          ((uint32_t)((c) & 0xff) << 8)  | \
          (uint32_t)((d) & 0xff)
#else
/** Set an IP address given by the four byte-parts.
    Little-endian version that prevents the use of htonl. */
#define HAL_NET_IPADDR(a,b,c,d) \
         ((uint32_t)((d) & 0xff) << 24) | \
         ((uint32_t)((c) & 0xff) << 16) | \
         ((uint32_t)((b) & 0xff) << 8)  | \
         (uint32_t)((a) & 0xff)
#endif


/** IP Address type */
typedef uint32_t hal_net_ipaddr_t;

/** Network interfaces */
typedef enum
{
   HAL_NETIF_ETH,
   HAL_NETIF_WIFI

} hal_netif_t;

/** WIFI MODE */
typedef enum
{
   HAL_NETIF_WIFI_MODE_CLIENT,
   HAL_NETIF_WIFI_MODE_AP

} hal_netif_wifi_mode_t;

/** Network interface configuration */
typedef struct
{
   struct
   {
        uint8_t macaddr[6];

   } eth;

   struct
   {
      hal_netif_wifi_mode_t mode;
      char ssid[32];
      char passwd[32];

   } wifi;

   uint8_t dhcp_enabled;

   hal_net_ipaddr_t ipaddr;
   hal_net_ipaddr_t gw;
   hal_net_ipaddr_t netmask;
   hal_net_ipaddr_t dns;

} hal_netif_config_t;


/** Events */
typedef enum
{
   HAL_NETIF_EVENT_CONNECTED,
   HAL_NETIF_EVENT_DISCONNECTED

} hal_netif_event_t;


/** Event handler type */
typedef void (*hal_net_event_handler_t)(hal_netif_t netif, hal_netif_event_t event);


int hal_net_init(void);

/** Configure network interface */
int hal_net_configure(hal_netif_t netif, const hal_netif_config_t *cfg, hal_net_event_handler_t event_handler);

/** Get local IP address */
const char *hal_net_get_local_ipaddr(hal_netif_t netif, char *buf, int bufsize);


#endif // __HAL_NET_H
