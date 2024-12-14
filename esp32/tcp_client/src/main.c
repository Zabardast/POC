
// tmp NVS
#include "nvs_flash.h"
#include "esp_wifi.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "string.h"

#include "setup_sta.c"

//task
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// static const char *STA_LOG_TAG = "WIFI_STATION ";

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

void tcp_send(void *data)
{
    ESP_LOGI(STA_LOG_TAG, "tcp_send() called");
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_size = sizeof(client_addr);

    //send hello to server
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == -1)
    {
        ESP_LOGE(STA_LOG_TAG, "socket creation error");
    }
    
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(4242);
    inet_pton(AF_INET, "10.0.0.218", &client_addr.sin_addr);

    if(connect(client_fd, (struct sockaddr *)&client_addr, client_size) < 0)
    {
        ESP_LOGE(STA_LOG_TAG, "connect error");
        close(client_fd);
    }

    // send!!
    send(client_fd, "hello from esp", 15 ,0);

    vTaskDelete(NULL);
}

void start_trafic(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI(STA_LOG_TAG, "start trafic event");
    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(STA_LOG_TAG, "creat task");
        xTaskCreatePinnedToCore(tcp_send, "send_tcp_hello", 4096, NULL, 1, NULL, 0);
        ESP_LOGI(STA_LOG_TAG, "start scheduler");
        // vTaskStartScheduler();
    }
}

EventGroupHandle_t eventGroup;

void app_main()
{

    vTaskDelay(4000);
    // int client_fd;
    // struct sockaddr_in client_addr;
    // socklen_t client_size = sizeof(client_addr);

    init_nvs();

    printf("start sta: \n");

    eventGroup = xEventGroupCreate();
    
    // setup sta
    init_station();

    // wait for IP || put the code below in a task that starts once an IP is aquired
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &start_trafic, NULL);

    // xTaskCreatePinnedToCore(tcp_send, "send_tcp_hello", 4096, NULL, 1, NULL, 0);

    // vTaskStartScheduler();

    // //send hello to server
    // client_fd = socket(AF_INET, SOCK_STREAM, 0);
    // if(client_fd == -1)
    // {
    //     ESP_LOGE(STA_LOG_TAG, "socket creation error");
    // }
    
    // client_addr.sin_family = AF_INET;
    // client_addr.sin_port = htons(4242);
    // inet_pton(AF_INET, "10.0.0.218", &client_addr.sin_addr);

    // if(connect(client_fd, (struct sockaddr *)&client_addr, client_size) < 0)
    // {
    //     ESP_LOGE(STA_LOG_TAG, "connect error");
    //     close(client_fd);
    // }

    // // send!!
    // send(client_fd, "hello", 5 ,0);

}