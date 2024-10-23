#include <stdio.h>

#include <string.h> // memset

#include "esp_wifi.h"

// tmp NVS
#include "nvs_flash.h"

#include "keys.c"

#define DEFAULT_SCAN_LIST_SIZE 20

void app_main()
{

    printf("start :\n");

    //----------------------------------------------------------------

    //  Initialize NVS

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //----------------------------------------------------------------

    // initialize wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    printf("esp init: \n");
    // esp_wifi_init uses NVS in osi_nvs_open() function which seems to be a custom function not meant for users.
    // I can only speculate that esp_wifi_init uses nvs to store configuration information.
    esp_wifi_init(&cfg);
    // set wifi mode to station (could be considered as a client where [Access Point]----->[station] the master or the provider is the AP)
    // enables STA mode (can be considered an interface)
    printf("Setting wifi mode/interface:\n");
    esp_wifi_set_mode(WIFI_MODE_STA);

    // wifi settings for the Station
    //https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_wifi.html?highlight=esp_wifi_set_mode#_CPPv417wifi_sta_config_t
    // better to populate it now because its a pain to edit // like with strncpy() or something
    wifi_config_t n_wifi_config = {
        .sta = {
            .ssid = {*AP_name},
            .password = {*pswd},
        },
    };

    // needs STA to be initialized
    esp_wifi_set_config(ESP_IF_WIFI_STA, &n_wifi_config);
    // starts wifi from the previous settings
    esp_err_t wf_result = esp_wifi_start();

    printf("Wifi start result: %i\n", wf_result);

    // scan networks in the area!!
    printf("start scanning networks : \n");
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
// return;
    //get results from scan
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE] = {0};
    uint16_t ap_count = 0;
    printf("ap_info : %u\n", sizeof(ap_info));
return;
    // memset(ap_info, 0, sizeof(ap_info));
// return;
    // UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL); // NULL for the current task
    // printf("High water mark for the current task: %u\n", stackHighWaterMark);
    esp_wifi_scan_get_ap_records(&number, ap_info);
    esp_wifi_scan_get_ap_num(&ap_count);

    printf("Found %u AP\n", number);

    // my AP name
    char good_to_connect = 0;

    for(int i = 0; i < number; i++)
    {
        printf("SSID : %s\n", ap_info[i].ssid);
        char *uintArrayStr = (char *)ap_info[i].ssid;

        if (!strcmp(uintArrayStr, AP_name))
        {
            //mac
            printf("mac: ");
            for (int mac_count = 0; mac_count < 6 ;mac_count++)
            {
                printf("%x ",ap_info[i].bssid[mac_count]);
            }
            printf("\n");
            printf("sig strength : %d\n", ap_info[i].rssi);
            // Bon ducoup on va se connecter.
            good_to_connect = 1;
            break;

        }
    }

    if(!good_to_connect) return;

    printf("connect to Ap\n");

    esp_err_t wf_connection = esp_wifi_connect();
    printf("Wifi connection result: %i\n", wf_connection);

    // use IwIp to ping my pc
    // make an arp map off all devices on the network.
    // can i see the traffic from other networks ?
    // what does a connection look like ? [Hacking]
    // can i transform my esp32 into a wifi extender for wpa3 ???[legit github project]
}