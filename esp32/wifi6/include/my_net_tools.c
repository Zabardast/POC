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

static const char *NAPT = "ip_forwarding";

void ip_forwarding();
void start_ping();

//static 
void wifi_station_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
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

        // try the ping before nat
        // start_ping();

        // setup nat
        ip_forwarding(event->ip_info.ip);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        printf("Connected!");

        ip_event_got_ip_t* ip_event_data = (ip_event_got_ip_t*) event_data;

        printf("start ip forwarding\n");
        ip_forwarding(ip_event_data->ip_info.ip);

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED) {
        printf("event IP_EVENT_AP_STAIPASSIGNED\n");

        ip_event_got_ip_t* ip_event_data = (ip_event_got_ip_t*) event_data;

        // ESP_LOGI(NAPT, "ip_napt_enabled for address " IPSTR, IP2STR(&ip_event_data));

    }
}


#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"

void wifi_sta_ap_event_ip_forwarding(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    printf("ip forwarding event\n");

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        printf("event ap sta connected!\n");

        // wifi_event_ap_staconnected_t
        // ip_event_got_ip_t

        wifi_event_ap_staconnected_t* ip_event_data = (wifi_event_ap_staconnected_t*) event_data;

        // Get the list of connected stations
        wifi_sta_list_t wifi_sta_list;
        // esp_netif_sta_list_t netif_sta_list;


        // tcpip_adapter_get_sta_list(wifi_sta_list, );
//
        // if (esp_wifi_ap_get_sta_list(&wifi_sta_list) == ESP_OK)
        // {
        //     if(wifi_sta_list.num > 0)
        //     {
        //         // wifi_sta_list->sta[0].mac
        //         wifi_sta_info_t *sta = &wifi_sta_list.sta[0];
        //         ESP_LOGI(NAPT, "Device %d: MAC:"MACSTR", RSSI: %d", 0, MAC2STR(sta->mac), sta->rssi);
        //     }
        // }
//

        // ESP_LOGI(NAPT, "ip_napt_enabled for address " IPSTR, IP2STR(&ip_event_data));

        // ip_forwarding(ip_event_data.ip_info.ip);

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED) {
        printf("event IP_EVENT_AP_STAIPASSIGNED\n");

        ip_event_got_ip_t* ip_event_data = (ip_event_got_ip_t*) event_data;

        // ESP_LOGI(NAPT, "ip_napt_enabled for address " IPSTR, IP2STR(ip_event_data->ip_info.ip.addr));

        ip_forwarding(ip_event_data->ip_info.ip);

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
    
    // esp_err_t crel_err = esp_event_loop_create_default();
    // if(crel_err != ESP_OK)
    // {
    //     printf("failed to create event loop");
    // }

    // add event handler for wifi and ip address change
    // esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_station_event_handler, NULL);
    // esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_station_event_handler, NULL);
    // esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_station_event_handler, NULL);

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

    esp_wifi_connect();

    // AP
     esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

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

    ///

    // esp_netif_dhcps_register_cb(ap_netif, dhcp_offer_callback);

    // esp_err_t hre = esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &wifi_sta_ap_event_ip_forwarding, NULL);
    // if (hre != ESP_OK)
    // {
    //     printf("Failed to register event handler\n");
    // }

}


void ip_forwarding(struct esp_ip4_addr p_addr) // or nat ???
{
    printf("ip_forwarding\n");
    // initial aproach ->> ip_napt_forward()

    ESP_LOGI(NAPT, "enable ip_napt for address " IPSTR, IP2STR(&p_addr));

    // ip_napt_enable(p_addr.addr, 1); // doesn't work lol

    // printf("ip_napt_enabled for address %i\n", IP2STR(&p_addr.addr));
    ESP_LOGI(NAPT, "ip_napt_enabled for address " IPSTR, IP2STR(&p_addr));

    // esp_netif_t * netif_sta = esp_netif_get_handle_from_ifkey("STA_HANDLE");

    // printf("va1\n");
    // // Get the IP address of the STA interface
    // esp_netif_ip_info_t ip_info;
    // esp_netif_get_ip_info(netif_sta, &ip_info);
    // heap_caps_defragment(MALLOC_CAP_32BIT);

    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

    printf("va2\n");
    // Enable NAPT on the STA IP address
    ip_napt_enable(p_addr.addr, 1);

    

}

#include "ping/ping_sock.h"

static const char *TAG = "8.8.8.8";

// Callback function to handle ping results
static void ping_results(esp_ping_handle_t *pf, void *args) {
    uint32_t elapsed_time;
    ip4_addr_t target_addr;
    uint8_t ttl;

    esp_log_level_set("wifi", ESP_LOG_DEBUG);
    esp_log_level_set("ping", ESP_LOG_DEBUG);
    
    printf("start ping results\n");
    // Get the ping response details
    // Check TIMEGAP (elapsed time)
    if (esp_ping_get_profile(*pf, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time)) == ESP_OK) {
        ESP_LOGI(TAG, "Ping time=%d ms", (int)elapsed_time);
    } else {
        ESP_LOGE(TAG, "Failed to retrieve ESP_PING_PROF_TIMEGAP");
    }

    // Check IPADDR (target address)
    if (esp_ping_get_profile(*pf, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr)) == ESP_OK) {
        ESP_LOGI(TAG, "Ping target IP=" IPSTR, IP2STR(&target_addr));
    } else {
        ESP_LOGE(TAG, "Failed to retrieve ESP_PING_PROF_IPADDR");
    }

    // Check TTL
    if (esp_ping_get_profile(*pf, ESP_PING_PROF_TTL, &ttl, sizeof(ttl)) == ESP_OK) {
        ESP_LOGI(TAG, "Ping TTL=%d", ttl);
    } else {
        ESP_LOGE(TAG, "Failed to retrieve ESP_PING_PROF_TTL");
    }


    ESP_LOGI(TAG, "Ping reply from " IPSTR ": time=%u ms, ttl=%u",
             IP2STR(&target_addr), (int)elapsed_time, (int)ttl);
}

void start_ping() {
    // Set up ping target
    ip_addr_t target_addr;
    ipaddr_aton("8.8.8.8", &target_addr);

    // Configure the ping parameters
    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;
    ping_config.count = 4;  // Number of ping attempts

    // Set up callback for ping response
    esp_ping_callbacks_t cbs = {
        .cb_args = NULL,
        .on_ping_success = ping_results,  // Called on each successful ping
        .on_ping_end = ping_results,      // Called when pinging ends
    };

    esp_ping_handle_t ping;
    esp_err_t err = esp_ping_new_session(&ping_config, &cbs, &ping);
    if (err == ESP_OK) {
        ESP_LOGE(TAG, "good new ping session");
        esp_err_t start_err = esp_ping_start(ping);
        if(start_err == ESP_OK) {
            ESP_LOGE(TAG, "esp ping start OK");
        }
    } else {
        ESP_LOGE(TAG, "Failed to start ping session");
    }
}


// void send_ping_to_host(const char *target_ip)
// {
//     printf("send_ping_to_host\n");

//     esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
//     ping_config.target_addr = inet_addr("8.8.8.8");  // Set the target IP address
//     ping_config.count = 4;                           // Number of ping requests to send

//     esp_ping_callbacks_t cbs = {
//         .cb_args = NULL,
//         .on_ping_success = ping_results,             // Called on each ping success
//         .on_ping_end = ping_results,                 // Called when pinging ends
//     };

//     esp_ping_handle_t ping;
//     esp_err_t err = esp_ping_new_session(&ping_config, &cbs, &ping);
//     if (err == ESP_OK) {
//         esp_ping_start(ping);
//     } else {
//         ESP_LOGE(TAG, "Failed to start ping session");
//     }
// }