
#include "system.h"

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"


TRACE_TAG(hal_net);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_HAL_NET_WIFI_MAX_STA_CONN
#define CFG_HAL_NET_WIFI_MAX_STA_CONN     4
#endif

#ifndef CFG_HAL_NET_WIFI_APNAME_FMT
#define CFG_HAL_NET_WIFI_APNAME_FMT       "AP-%2.2X:%2.2X:%2.2X"
#endif

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static ip4_addr_t s_ip_addr;
static hal_netif_t hal_netif;
static hal_net_event_handler_t hal_event_handler;

// Prototypes:
static esp_err_t event_handler(void *ctx, system_event_t *event);


int hal_net_init(void)
{
   return 0;
}

/** Configure network interface */
int hal_net_configure(hal_netif_t netif, const hal_netif_config_t *cfg, hal_net_event_handler_t _hal_event_handler)
{
   hal_netif = netif;
   hal_event_handler = _hal_event_handler;

   switch(netif)
   {
      case HAL_NETIF_WIFI:
      {
         switch(cfg->wifi.mode)
         {
            case HAL_NETIF_WIFI_MODE_CLIENT:
            {
               wifi_config_t wifi_config;
               tcpip_adapter_ip_info_t ipinfo;
               wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();

               memset(&wifi_config, 0, sizeof(wifi_config_t));
               strlcpy((char *)wifi_config.sta.ssid, cfg->wifi.ssid, sizeof(wifi_config.sta.ssid));
               strlcpy((char *)wifi_config.sta.password, cfg->wifi.passwd, sizeof(wifi_config.sta.password));

               tcpip_adapter_init();

               if (!cfg->dhcp_enabled)
               {
                  tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);

                  ipinfo.ip.addr = cfg->ipaddr;
                  ipinfo.gw.addr = cfg->gw;
                  ipinfo.netmask.addr = cfg->netmask;

                  if (tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipinfo) != ESP_OK)
                  {
                      TRACE_ERROR("Static IP address configuration failed");
                      return -1;
                  }

                  TRACE("Static IP address configuration, IP: %s", ip4addr_ntoa(&ipinfo.ip));
               }
               else
               {
                  TRACE("DHCP client enabled");
               }

               wifi_event_group = xEventGroupCreate();
               ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

               ESP_ERROR_CHECK(esp_wifi_init(&init_cfg) );
               ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM) );

               ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
               ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

               ESP_ERROR_CHECK(esp_wifi_start());
               ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

               TRACE("Connecting to '%s'  passwd: %s", wifi_config.sta.ssid, wifi_config.sta.password);

               // Wait for wifi is connected
               //xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
            }
            break;

            case HAL_NETIF_WIFI_MODE_AP:
            {
               uint8_t mac[6];
               wifi_config_t wifi_config =
               {
                  .ap = {
                     .max_connection = CFG_HAL_NET_WIFI_MAX_STA_CONN,
                     .authmode = WIFI_AUTH_WPA_WPA2_PSK
                  },
               };

               strcpy(wifi_config.ap.password, cfg->wifi.passwd);

               wifi_event_group = xEventGroupCreate();

               tcpip_adapter_init();
               ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

               wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
               ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

               // Generate SSID from mac
               ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_AP, mac));
               wifi_config.ap.ssid_len = snprintf((char *)wifi_config.ap.ssid, sizeof(wifi_config.ap.ssid), CFG_HAL_NET_WIFI_APNAME_FMT, mac[3], mac[4], mac[5]);

               if (strlen(cfg->wifi.passwd) == 0)
               {
                  wifi_config.ap.authmode = WIFI_AUTH_OPEN;
               }

               ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
               ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
               ESP_ERROR_CHECK(esp_wifi_start());

               TRACE("Wifi_init_softap finished.SSID:%s password:%s", (char *)wifi_config.ap.ssid, cfg->wifi.passwd);

               if (hal_event_handler != NULL)
                  hal_event_handler(hal_netif, HAL_NETIF_EVENT_CONNECTED);
            }
            break;
         }
      }
      break;

      default:
         TRACE_ERROR("Not supported netif: %d", netif);
         return -1;
   }

   return 0;
}


const char *hal_net_get_local_ipaddr(hal_netif_t netif, char *buf, int bufsize)
{
   strcpy(buf, ip4addr_ntoa(&s_ip_addr));
   return buf;
}


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
   switch (event->event_id)
   {
      case SYSTEM_EVENT_STA_START:
         esp_wifi_connect();
         break;

      case SYSTEM_EVENT_STA_GOT_IP:
         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
         s_ip_addr = event->event_info.got_ip.ip_info.ip;

         TRACE("Connected, IP address: %s", ip4addr_ntoa(&s_ip_addr));

         if (hal_event_handler != NULL)
            hal_event_handler(hal_netif, HAL_NETIF_EVENT_CONNECTED);
         break;

     case SYSTEM_EVENT_AP_STACONNECTED:
         TRACE("Station:"MACSTR" join, AID=%d", MAC2STR(event->event_info.sta_connected.mac), event->event_info.sta_connected.aid);
         break;

      case SYSTEM_EVENT_AP_STADISCONNECTED:
         TRACE("Station:"MACSTR"leave, AID=%d", MAC2STR(event->event_info.sta_disconnected.mac), event->event_info.sta_disconnected.aid);
         break;

      case SYSTEM_EVENT_STA_DISCONNECTED:
         /* This is a workaround as ESP32 WiFi libs don't currently
          auto-reassociate. */
         esp_wifi_connect();
         xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
         if (hal_event_handler != NULL)
            hal_event_handler(hal_netif, HAL_NETIF_EVENT_DISCONNECTED);
         break;
      default:
         break;
   }
   return ESP_OK;
}
