#include <stdio.h>

#include <string.h> // memset

#include "esp_wifi.h"

// tmp NVS
#include "nvs_flash.h"

#include "keys.c"

#include "my_net_tools.c"

#define DEFAULT_SCAN_LIST_SIZE 10

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
    esp_log_level_set("wifi", ESP_LOG_DEBUG);
    // connect to wifi
    init_station();

    // use IwIp to ping my pc
    // const char *target_ip = "10.0.0.218";
    // send_ping_to_host(target_ip);


    // make an arp map off all devices on the network.
    // can i see the traffic from other networks ?
    // what does a connection look like ? [Hacking]
    // can i transform my esp32 into a wifi extender for wpa3 ???[legit github project]
}