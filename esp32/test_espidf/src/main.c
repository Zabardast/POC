

// #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
// #include "esp_log.h"

// static const char* TAG = "dev_tag";
#include <stdio.h>

#include "esp_system.h"

void app_main(void)
{

    // esp_log_level_set("lib_name", ESP_LOG_WARN);        // enables WARN logs from lib_name
    // esp_log_set_level_master(ESP_LOG_NONE);             // disables all logs globally. esp_log_level_set has no effect at the moment.

    // ESP_LOGW(TAG, "hello world", error * 100, baud_req, baud_real);
    while (1) {
        printf("hello world\n");
    }

    esp_restart();

}