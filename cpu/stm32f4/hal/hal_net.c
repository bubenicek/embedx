
#include "system.h"

#if defined(CFG_USE_LWIP)

#include "netif/ethernetif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"

TRACE_TAG(hal_net);

#define SERVICE_PAGE_MAC_ADDR0         0x7C
#define SERVICE_PAGE_MAC_ADDR1         0x1E
#define SERVICE_PAGE_MAC_ADDR2         0xB3

// Prototypes:
static void network_status_cb(struct netif *netif);

/** Network interface */
static struct netif netif;

int hal_net_init(void)
{
   // Create TCP/IP stack thread
   tcpip_init(NULL, NULL);

   return 0;
}

int hal_net_configure(hal_netif_t hal_netif, const hal_netif_config_t *cfg, hal_net_event_handler_t event_handler)
{
   uint32_t uuid;
   ip_addr_t ipaddr = {0};
   ip_addr_t netmask = {0};
   ip_addr_t gw = {0};

   IP4_ADDR(&ipaddr, 10,10,10,54);
   IP4_ADDR(&netmask, 255,255,255,0);
   IP4_ADDR(&gw, 10,10,10,1);

   uuid = hal_get_board_uuid32();

   // Make unique ethernet address
   netif.hwaddr[0] = SERVICE_PAGE_MAC_ADDR0;
   netif.hwaddr[1] = SERVICE_PAGE_MAC_ADDR1;
   netif.hwaddr[2] = (uuid >> 24) & 0xFF;
   netif.hwaddr[3] = (uuid >> 16) & 0xFF;
   netif.hwaddr[4] = (uuid >> 8) & 0xFF;
   netif.hwaddr[5] = (uuid & 0xFF);

   // Configure net interface
   if (netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input) == NULL)
   {
      TRACE_ERROR("Init netif failed");
      return -1;
   }

   // Setup callback function for netif status change
   netif_set_status_callback(&netif, network_status_cb);

   // Registers the default network interface.
   netif_set_default(&netif);

   if (cfg->dhcp_enabled)
   {
      // Start DHCP
      dhcp_start(&netif);

      TRACE("Net if init, MAC addr: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
            netif.hwaddr[0], netif.hwaddr[1], netif.hwaddr[2], netif.hwaddr[3], netif.hwaddr[4], netif.hwaddr[5]);
   }
   else
   {
      TRACE("Net if init, MAC addr: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X  IP: %s",
            netif.hwaddr[0], netif.hwaddr[1], netif.hwaddr[2], netif.hwaddr[3], netif.hwaddr[4], netif.hwaddr[5], ipaddr_ntoa(&ipaddr));
   }

   return 0;
}


const char *hal_net_local_ipaddr(char *buf, int bufsize)
{
   snprintf(buf, bufsize - 1, "%s", ipaddr_ntoa(&netif.ip_addr));
   return buf;
}

static void network_status_cb(struct netif *netif)
{
   if (netif_is_up(netif))
   {
      TRACE("Eth network is UP");
      TRACE("   MAC     = %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", netif->hwaddr[0], netif->hwaddr[1], netif->hwaddr[2], netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]);
      TRACE("   ip_addr = %s", ipaddr_ntoa(&netif->ip_addr));
      TRACE("   netmask = %s", ipaddr_ntoa(&netif->netmask));
      TRACE("   gw      = %s", ipaddr_ntoa(&netif->gw));
   }
   else
   {
      TRACE("Eth network is DOWN");
   }
}

#endif   // CFG_USE_LWIP