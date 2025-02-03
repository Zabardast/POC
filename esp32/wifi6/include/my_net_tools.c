#include "esp_netif.h"


#include <string.h>
#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"


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
// #endif

#include "lwip/ip4_napt.h"

//
#include "nat.c"


#define DEFAULT_SCAN_LIST_SIZE 10
static const char *STA_WIFI = "wifi STA";
static const char *NAPT = "ip_forwarding";

#include <sys/socket.h>
// static const char *STA_wireshark = "tcp for wireshark";
#define PORT 4242

void ip_forwarding();
void initialize_ping();

#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"


void wifi_sta_lost_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    printf("ip forwarding event\n");

    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        // reconnect
        printf("esp_wifi_connect : %i\n",esp_wifi_connect());
    }
}


void ip_forwarding(struct esp_ip4_addr p_addr) // or nat ???
{
    printf("ip_forwarding\n");

    // debug ?? TODO: remove this
    // heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

    ESP_LOGI(NAPT, "enable ip_napt for address " IPSTR, IP2STR(&p_addr));
    // printf("ip address: %i\n", (int)p_addr.addr);

    // Enable NAPT on the STA IP address
    ip_napt_enable(ip4_addr_get_u32(&p_addr), 1);

    ESP_LOGI(NAPT, "ip_napt_enabled for address " IPSTR, IP2STR(&p_addr));
    
}

///

// void wifi_sta_enable_napt(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
void wifi_sta_enable_napt(void * arg)
{
    // if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        esp_netif_t *sta_netif = (esp_netif_t *)arg; // STA interface, not AP

        // Get STA IP info
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(sta_netif, &ip_info);

        // Log STA IP address
        ESP_LOGI(NAPT, "STA IP Address: " IPSTR, IP2STR(&ip_info.ip));

        if (ip_info.ip.addr != 0) {
            // Enable NAPT on the STA interface
            esp_err_t enab_nat_err = esp_netif_napt_enable(sta_netif);
            if (enab_nat_err == ESP_OK) {
                ESP_LOGI(NAPT, "NAPT successfully enabled on STA interface");

                // Enable IP forwarding for the STA IP address
                ip_napt_enable(ip4_addr_get_u32(&ip_info.ip), 1);
            } else {
                ESP_LOGE(NAPT, "Failed to enable NAPT: %s", esp_err_to_name(enab_nat_err));
            }
        } else {
            ESP_LOGW(NAPT, "No IP assigned to STA interface, cannot enable NAPT.");
        }
    // }
}

///


// void wifi_sta_enable_napt(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
// {
//     if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
//     {
//         ESP_LOGI("NAPT","Trying to enable NAPT");

//         esp_netif_t *sta_netif = (esp_netif_t *) arg;

//         esp_netif_ip_info_t ip_info;
//         esp_netif_get_ip_info(sta_netif, &ip_info);

//         if (ip_info.ip.addr != 0) {
//             esp_err_t enab_nat_err = esp_netif_napt_enable(sta_netif);
//             if (enab_nat_err == ESP_OK) {
//                 ESP_LOGI("NAPT", "NAPT successfully enabled");
//                 esp_netif_ip_info_t ip_info;
//                 esp_netif_get_ip_info((esp_netif_t *)arg, &ip_info);


//                 printf("frw ip : \n");

//                 ESP_LOGI(NAPT, "forward ip :" IPSTR, IP2STR(&ip_info.ip));
//                 ip_forwarding(ip_info.ip);
//             } else {
//                 ESP_LOGE("NAPT", "Failed to enable NAPT: %s", esp_err_to_name(enab_nat_err));
//             }
//         } else {
//             ESP_LOGW("NAPT", "No IP assigned yet, cannot enable NAPT.");
//         }
//     }
// }

// void wifi_sta_ap_event_ip_forwarding(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
// {
//     printf("ip forwarding event\n");

//     if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
//     {
//         printf("event IP_EVENT_STA_GOT_IP\n");

//         esp_netif_ip_info_t ip_info;
//         esp_netif_get_ip_info((esp_netif_t *)arg, &ip_info);


//         printf("frw ip : \n");

//         ESP_LOGI(NAPT, "forward ip :" IPSTR, IP2STR(&ip_info.ip));
//         ip_forwarding(ip_info.ip);

//     }
// }

void setup_ap(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

        wifi_config_t wifi_config_ap = {
            .ap = {
                .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
                .ssid = EXAMPLE_ESP_WIFI_SSID,
                .password = EXAMPLE_ESP_WIFI_PASS,
                .channel = EXAMPLE_ESP_WIFI_CHANNEL,
                .max_connection = EXAMPLE_MAX_STA_CONN,
                // .authmode = WIFI_AUTH_WPA_WPA2_PSK
                .authmode = WIFI_AUTH_WPA2_WPA3_PSK
            },
        };
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap);

        // esp_netif_ip_info_t ip_info;
        // esp_netif_get_ip_info(sta_netif, &ip_info);


        // esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_sta_enable_napt, arg); //ap_netif
        // esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_sta_ap_event_ip_forwarding, sta_netif);

        esp_err_t ap_result = esp_wifi_start();

        printf("Wifi start ap result: %i\n", ap_result);

        wifi_sta_enable_napt(arg);
    }
}

void init_AP_STA()
{
    // first the sta to connect to provider AP
    esp_netif_init();

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    
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

    esp_err_t sta_result = esp_wifi_start();

    printf("Wifi start sta result: %i\n", sta_result);

    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, setup_ap, sta_netif); //ap_netif

    esp_err_t sta_connect = esp_wifi_connect();
    if(sta_connect == ESP_OK)
    {
        ESP_LOGI(STA_WIFI, "Wifi sta connected successfully\n");
    }else{
        ESP_LOGI(STA_WIFI, "Wifi sta failed to connect\n");
    }

    // AP
    // esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

    // wifi_config_t wifi_config_ap = {
    //     .ap = {
    //         .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
    //         .ssid = EXAMPLE_ESP_WIFI_SSID,
    //         .password = EXAMPLE_ESP_WIFI_PASS,
    //         .channel = EXAMPLE_ESP_WIFI_CHANNEL,
    //         .max_connection = EXAMPLE_MAX_STA_CONN,
    //         // .authmode = WIFI_AUTH_WPA_WPA2_PSK
    //         .authmode = WIFI_AUTH_WPA2_WPA3_PSK
    //     },
    // };
    // esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap);

    // // esp_netif_ip_info_t ip_info;
    // // esp_netif_get_ip_info(sta_netif, &ip_info);


    // esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_sta_enable_napt, sta_netif); //ap_netif
    // // esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_sta_ap_event_ip_forwarding, sta_netif);

    // esp_err_t ap_result = esp_wifi_start();

    // printf("Wifi start ap result: %i\n", ap_result);

    // ESP_LOGI(NAPT, "STA IP :" IPSTR, IP2STR(&ip_info.ip));

    // esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_sta_ap_event_ip_forwarding, &ap_netif);
    // esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &wifi_sta_ap_event_ip_forwarding, NULL);

}

