#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_http_client.h"

#include "keys.c"

static const char *STA_LOG_TAG = "WIFI_STATION ";

void station_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_id == IP_EVENT_STA_GOT_IP) {
        
        ip_event_got_ip_t* ip_event_data = (ip_event_got_ip_t*) event_data;
        
        ESP_LOGI(STA_LOG_TAG, "new ip : "IPSTR , IP2STR(&ip_event_data->ip_info.ip));
    } else {
        ESP_LOGI(STA_LOG_TAG, "no action for this event_id");
    }
}

void station_connect(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_id == IP_EVENT_STA_LOST_IP) {
        ESP_LOGI(STA_LOG_TAG, "lost ip -> retrying...");
        esp_wifi_connect();
    } else if(event_id == WIFI_EVENT_STA_DISCONNECTED){
        ESP_LOGI(STA_LOG_TAG, "disconnected -> retrying...");
        ESP_LOGI(STA_LOG_TAG," %i" ,((wifi_event_sta_disconnected_t*)event_data)->reason);
        //WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD  = 211,
        if(((wifi_event_sta_disconnected_t*)event_data)->reason == 211)
            ESP_LOGI(STA_LOG_TAG, "211 == WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD");
        esp_wifi_connect();
    } else if(event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(STA_LOG_TAG, "start connection -> connectiong...");
        esp_wifi_connect();
    } else {
        ESP_LOGI(STA_LOG_TAG, "no action for this event_id");
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

    //
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &station_connect, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &station_got_ip, NULL);
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &station_connect, NULL);


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
        },
    };


    // needs STA to be initialized
    esp_wifi_set_config(ESP_IF_WIFI_STA, &n_wifi_config);
    // starts wifi from the previous settings
    esp_err_t wf_result = esp_wifi_start();

    printf("Wifi start result: %i\n", wf_result);

    esp_err_t wf_connect_result = esp_wifi_connect();

    if( wf_connect_result == ESP_OK)
    {
        ESP_LOGI(STA_LOG_TAG, "esp_wifi_connect OK");
    } else if( wf_connect_result == ESP_ERR_WIFI_SSID)
    {
        ESP_LOGI(STA_LOG_TAG, "esp_wifi_connect wrong SSID");
    } else if( wf_connect_result == ESP_ERR_WIFI_CONN)
    {
        ESP_LOGI(STA_LOG_TAG, "esp_wifi_connect internal error");
    }

}