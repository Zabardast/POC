#include <stdio.h>

#include <string.h> // memset

#include "esp_wifi.h"

// tmp NVS
#include "nvs_flash.h"

#include "keys.c"

#include "my_net_tools.c"

#define DEFAULT_SCAN_LIST_SIZE 10

void init_nvs()
{
    //  Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void got_ip()
{
    printf("event got ip\n");
}

void app_main()
{

    init_nvs();

    esp_err_t crel_err = esp_event_loop_create_default();
    if(crel_err != ESP_OK)
    {
        printf("failed to create event loop");
    }

    printf("start wifi :\n");
    esp_log_level_set("wifi", ESP_LOG_DEBUG);

    // event handlers
    esp_event_handler_instance_register(IP_EVENT_STA_GOT_IP, ESP_EVENT_ANY_ID, &got_ip, NULL, NULL);
    
    esp_err_t hre = esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &wifi_sta_ap_event_ip_forwarding, NULL);
    if (hre != ESP_OK)
    {
        printf("Failed to register event handler\n");
    }


    // Be a AP_STA with ip forwarding
    init_AP_STA();


    //----------------------------------------------------------------



    //----------------------------------------------------------------
    
    // Be a STA
    // init_station();


    // Be a AP
    // init_access_point();


    // Be a AP_STA with ip forwarding
    // init_AP_STA();

    // start_ping();
    // ip_forwarding();
}