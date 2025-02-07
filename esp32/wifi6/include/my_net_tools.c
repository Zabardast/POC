// #include "esp_netif.h"
// #include "esp_napt.h"

#include <string.h>
#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/ip4_napt.h"


#include "lwip/init.h"
#include "lwip/ip_addr.h" // Include this header for IP4_ADDR
#include "lwip/tcpip.h"
#include "lwip/lwip_napt.h"
#include "lwip/sockets.h"

#include "lwip/opt.h"

#include "esp_wifi.h"
#include "esp_log.h"

//

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
// #include "esp_wpa2.h" // deprecated
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "lwip/opt.h"



#include "lwip/err.h"
#include "lwip/sys.h"

// enable ip forwarding and napt in menuconfig
// #ifdef IP_NAPT
#include "lwip/lwip_napt.h"
#include "lwip/dns.h"
// #endif

// #include "lwip/ip4_napt.h"

//
#include "nat.c"


#define DEFAULT_SCAN_LIST_SIZE 10
static const char *STA_WIFI = "wifi STA";
static const char *AP_WIFI = "wifi AP";
static const char *WIFI_TAG = "wifi TAG";
static const char *NAPT = "wifi NAPT";

#define DHCPS_OFFER_DNS 0x02

#include <sys/socket.h>

#define PORT 4242

void ip_forwarding();
void initialize_ping();

#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"

esp_netif_t *ap_netif;
esp_netif_t *sta_netif;


void print_network_interfaces()
{
    esp_netif_t *netif = NULL;
    while((netif = esp_netif_next(netif)) != NULL)
    {
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(netif, &ip_info);
        ESP_LOGI(NAPT, "Interface: %p, IP: " IPSTR, netif, IP2STR(&ip_info.ip));
    }
}

typedef struct {
    uint32_t ip_addr;
} napt_args_t;


static void napt_enable_cb(void *ctx)
{
    ESP_LOGI(NAPT, "NAPT enable in TCP/IP thread");

    napt_args_t *args = (napt_args_t *)ctx;
    if (args != NULL) {
        ip_napt_enable(args->ip_addr, 1);
        free(args);  // Free the allocated structure

        
        ESP_LOGI(NAPT, "NAPT enabled in TCP/IP thread");
    }
}

void softap_set_dns_addr()
{
    esp_netif_dns_info_t dns;
    esp_netif_get_dns_info(sta_netif,ESP_NETIF_DNS_MAIN,&dns);
    uint8_t dhcps_offer_option = DHCPS_OFFER_DNS;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(ap_netif));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_offer_option, sizeof(dhcps_offer_option)));
    ESP_ERROR_CHECK(esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(ap_netif));
}

void enable_nat(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {

        // Add these debug prints in your IP_EVENT_STA_GOT_IP handler
        // esp_netif_ip_info_t sta_ip_info;
        // esp_netif_ip_info_t ap_ip_info;
        // esp_netif_get_ip_info(sta_netif, &sta_ip_info);
        // esp_netif_get_ip_info(ap_netif, &ap_ip_info);

        // ESP_LOGI(WIFI_TAG, "STA IP: " IPSTR ", GW: " IPSTR, IP2STR(&sta_ip_info.ip), IP2STR(&sta_ip_info.gw));
        // ESP_LOGI(WIFI_TAG, "AP  IP: " IPSTR ", GW: " IPSTR, IP2STR(&ap_ip_info.ip), IP2STR(&ap_ip_info.gw));

        ESP_LOGI(NAPT, "START ENABLE NAT FOR STA");
        esp_netif_t *sta_netif = (esp_netif_t *)arg; // STA interface, not AP

        // Get STA IP info
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(sta_netif, &ip_info);

        // Log STA IP address
        ESP_LOGI(NAPT, "STA IP Address: " IPSTR, IP2STR(&ip_info.ip));

        if (ip_info.ip.addr != 0) {
            
            // Enable NAPT on the STA interface

            napt_args_t *args = malloc(sizeof(napt_args_t));
            if (args == NULL) {
                ESP_LOGE(NAPT, "Failed to allocate memory for NAPT args");
                return;
            }
            args->ip_addr = ip_info.ip.addr;

            
            esp_err_t err = tcpip_callback(napt_enable_cb, args);
            if (err != ESP_OK) {
                ESP_LOGE(NAPT, "Failed to schedule NAPT enable: %s", esp_err_to_name(err));
                free(args);
                return;
            }
            // ip_napt_enable(ip_info.ip.addr, 1);
            
            
            // esp_err_t enab_nat_err = esp_netif_napt_enable(sta_netif);
            // if (enab_nat_err == ESP_OK) {
            //     ESP_LOGI(NAPT, "NAPT successfully enabled on STA interface");

            //     // Enable IP forwarding for the STA IP address
            //     // ip_napt_enable(ip4_addr_get_u32(&ip_info.ip), 1);
            //     ip_napt_enable(ip_info.ip.addr, 1);
            // } else {
            //     ESP_LOGE(NAPT, "Failed to enable NAPT: %s", esp_err_to_name(enab_nat_err));
            // }
        } else {
            ESP_LOGW(NAPT, "No IP assigned to STA interface, cannot enable NAPT.");
        }
        print_network_interfaces();
        ESP_LOGI(NAPT, "Setup DNS configuration");
        
        softap_set_dns_addr();

        esp_netif_ip_info_t sta_ip_info, ap_ip_info;
        esp_netif_get_ip_info(sta_netif, &sta_ip_info);
        esp_netif_get_ip_info(ap_netif, &ap_ip_info);
        ESP_LOGI(NAPT, "Routes - STA: " IPSTR "/24 via " IPSTR, IP2STR(&sta_ip_info.ip), IP2STR(&sta_ip_info.gw));
        ESP_LOGI(NAPT, "Routes - AP: " IPSTR "/24 via " IPSTR, IP2STR(&ap_ip_info.ip), IP2STR(&ap_ip_info.gw));
    }
}

void init_AP_STA()
{
    // first the sta to connect to provider AP
    esp_netif_init();

    sta_netif = esp_netif_create_default_wifi_sta();
    
    assert(sta_netif);

    //setup
    // STA
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_netif_set_hostname(sta_netif, "STA_HANDLE");

    esp_wifi_set_mode(WIFI_MODE_APSTA);

    //config
    wifi_config_t wifi_config_sta = {
        .sta = {
            .ssid = AP_NAME,
            .password = PSWD,
        },
    };
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta);

    ap_netif = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_WPA3_PSK
        },
    };
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap);

    esp_err_t ap_result = esp_wifi_start();

    if (ap_result != ESP_OK)
    {
        ESP_LOGE(AP_WIFI, "WiFi AP start failed: %s", esp_err_to_name(ap_result));
    }else{
        ESP_LOGI(AP_WIFI, "WiFi AP started: %s", esp_err_to_name(ap_result));
    }

    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, enable_nat, sta_netif);

    esp_err_t sta_connect = esp_wifi_connect();
    if(sta_connect == ESP_OK)
    {
        ESP_LOGI(STA_WIFI, "Wifi sta connected successfully\n");
    }else{
        ESP_LOGI(STA_WIFI, "Wifi sta failed to connect\n");
    }

}

