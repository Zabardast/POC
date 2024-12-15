
#include "esp_wifi.h"
#include "esp_log.h"

// #include "esp_netif.h"

#include "keys.c"

static const char *AP_LOG_TAG = "WIFI_AP ";

void new_device(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_id == IP_EVENT_AP_STAIPASSIGNED) {

        // print new device ip
        ip_event_ap_staipassigned_t* ip_event_data = (ip_event_ap_staipassigned_t*) event_data;

        esp_netif_pair_mac_ip_t pair_mac_ip = {0};
        memcpy(pair_mac_ip.mac, ip_event_data->mac, sizeof(pair_mac_ip.mac));

        if( esp_netif_dhcps_get_clients_by_mac(arg, 1, &pair_mac_ip) == ESP_OK) {
        
            ESP_LOGI(AP_LOG_TAG, "new ap sta : " IPSTR, IP2STR(&pair_mac_ip.ip));
        }

    } else {

        ESP_LOGI(AP_LOG_TAG, "no action for this event_id");
    }
}

void init_access_point()
{
    esp_netif_init();
    esp_event_loop_create_default();

    esp_netif_t* ap_netif = esp_netif_create_default_wifi_ap();

    esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, new_device, ap_netif);


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);


    esp_wifi_set_mode(WIFI_MODE_AP);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA3_PSK
        },
    };
    
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
    
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);

    esp_wifi_start();
}
