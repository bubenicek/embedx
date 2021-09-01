
#include "system.h"

#include <sys/ioctl.h>
#include <net/if.h>

#if ENABLE_TRACE_HAL
TRACE_TAG(hal_net);
#else
#include "trace_undef.h"
#endif

#ifndef CFG_HAL_NETIF_ETH_NAME
#define CFG_HAL_NETIF_ETH_NAME      "eth0"
#endif

#ifndef CFG_HAL_NETIF_WIFI_NAME
#define CFG_HAL_NETIF_WIFI_NAME      "wlan0"
#endif

int hal_net_init(void)
{
   return 0;
}

/** Configure network interface */
int hal_net_configure(hal_netif_t netif, const hal_netif_config_t *cfg, hal_net_event_handler_t event_handler)
{
    if (event_handler != NULL)
        event_handler(netif, HAL_NETIF_EVENT_CONNECTED);

   return 0;
}

const char *hal_net_get_local_ipaddr(hal_netif_t netif, char *buf, int bufsize)
{
   int fd = -1;
   struct ifreq ifr;
   const char *netif_name;

   switch(netif)
   {
      case HAL_NETIF_ETH:
         netif_name = CFG_HAL_NETIF_ETH_NAME;
         break;

      case HAL_NETIF_WIFI:
         netif_name = CFG_HAL_NETIF_WIFI_NAME;
         break;

      default:
         TRACE_ERROR("Not supported network interface");
         throw_exception(fail);
   }


   if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
      throw_exception(fail);

   // I want to get an IPv4 IP address
   ifr.ifr_addr.sa_family = AF_INET;

   // I want IP address attached
   strncpy(ifr.ifr_name, netif_name, IFNAMSIZ-1);

   if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
   {
      TRACE_ERROR("Get local IP address of %s failed", netif_name);
      throw_exception(fail);
   }

   close(fd);
   snprintf(buf, bufsize, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

   return buf;

fail:
   if (fd != -1)
      close(fd);

   return NULL;
}
