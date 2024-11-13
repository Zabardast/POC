
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

void ip_forwarding();

static void wifi_station_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        printf("esp_wifi_connect : %i\n",esp_wifi_connect());
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        printf("Disconnected from Wi-Fi, reason: %d\n", event->reason);

        printf("esp_wifi_connect : %i\n",esp_wifi_connect());
        printf("Disconnected from Wi-Fi, retrying...\n");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("Connected! Got IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        // Now you can send packets, as the IP address is assigned
        // setup nat
        ip_forwarding(event->ip_info.ip);
    }
}



void init_station()
{
    //new
// Initialize the network interface
    esp_netif_init();
    esp_event_loop_create_default();

// Set up the Wi-Fi station interface
    esp_netif_create_default_wifi_sta();

    //end new

    // initialize wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    printf("esp init: \n");
    // esp_wifi_init uses NVS in osi_nvs_open() function which seems to be a private function not meant for users.
    // I can only speculate that esp_wifi_init uses nvs to store configuration information.
    esp_wifi_init(&cfg);

    // add event handler for wifi and ip address change
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_station_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_station_event_handler, NULL);


    // set wifi mode to station (could be considered as a client where [Access Point]----->[station] the master or the provider is the AP)
    // enables STA mode (can be considered an interface)
    printf("Setting wifi mode/interface:\n");
    esp_wifi_set_mode(WIFI_MODE_STA);

    // wifi settings for the Station
    wifi_config_t n_wifi_config = {
        .sta = {
            .ssid = AP_NAME,
            .password = PSWD,
            .threshold.authmode = WIFI_AUTH_WPA3_PSK,
            .scan_method = WIFI_FAST_SCAN,
            .pmf_cfg = {
                .capable = true,
                .required = true
            },
            // .bssid_set = false,
            // .channel = 6
        },
    };


    // needs STA to be initialized
    esp_wifi_set_config(ESP_IF_WIFI_STA, &n_wifi_config);
    // starts wifi from the previous settings
    esp_err_t wf_result = esp_wifi_start();

    printf("Wifi start result: %i\n", wf_result);

}


void init_access_point()
{
    // // tmp


    // //
    esp_netif_init();
    esp_event_loop_create_default();

    esp_netif_create_default_wifi_ap();

    // // esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);



    // setup the wifi handlers to manage connections to the AP


    // esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);

    esp_wifi_set_mode(WIFI_MODE_AP);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);

    esp_wifi_start();
}


void init_AP_STA()
{
    // first the sta to connect to provider AP
    esp_netif_init();
    esp_event_loop_create_default();

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    //setup
    // STA
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // add event handler for wifi and ip address change
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_station_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_station_event_handler, NULL);

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

    esp_wifi_connect();

    // AP
    esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap);

    esp_err_t ap_result = esp_wifi_start();

    printf("Wifi start ap result: %i\n", ap_result);

}


void ip_forwarding(struct esp_ip4_addr p_addr) // or nat ???
{
    printf("ip_forwarding yo\n");
    // initial aproach ->> ip_napt_forward()

    ip_napt_enable(p_addr.addr, 1); // doesn't work lol

    // esp_netif_t * netif_sta = esp_netif_get_handle_from_ifkey("STA_HANDLE");

    // printf("va1\n");
    // // Get the IP address of the STA interface
    // esp_netif_ip_info_t ip_info;
    // esp_netif_get_ip_info(netif_sta, &ip_info);

    // printf("va2\n");
    // Enable NAPT on the STA IP address
    // ip_napt_enable(ip_info.ip.addr, 1);

    

}

// void send_ping_to_host(const char *target_ip)
// {
//     printf("send_ping_to_host\n");
// }