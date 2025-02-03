
// tmp NVS
#include "nvs_flash.h"
#include "esp_wifi.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "string.h"

#include "setup_sta.c"

#include "esp_tls.h"

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
        ESP_LOGE(STA_LOG_TAG, "socket creation error");
    
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(4242);
    // PC ip from router
    // inet_pton(AF_INET, "10.0.0.218", &client_addr.sin_addr);
    // PC ip from esp32 AP
    inet_pton(AF_INET, "192.168.4.4", &client_addr.sin_addr);

    if(connect(client_fd, (struct sockaddr *)&client_addr, client_size) < 0)
    {
        ESP_LOGE(STA_LOG_TAG, "connect error");
        close(client_fd);
    }

    // send!!
    send(client_fd, "hello from esp", 15 ,0);

    vTaskDelete(NULL);
}

// HTTP event handler
esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Optional: Print response data if needed
            ESP_LOGI(STA_LOG_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void init_global_ca_store()
{
    // init CA store
    esp_err_t ca_err = esp_tls_init_global_ca_store();
    if(ca_err != ESP_OK)
    {
        ESP_LOGI(STA_LOG_TAG, "CA store init NOK !\n");
        return;
    }

    extern const uint8_t ca_cert_bundle_start[] asm("_binary_cacert_all_pem_start");
    extern const uint8_t ca_cert_bundle_end[] asm("_binary_cacert_all_pem_end");
    size_t bundle_size = ca_cert_bundle_end - ca_cert_bundle_start;

    ca_err = esp_tls_set_global_ca_store(ca_cert_bundle_start, bundle_size);
    if(ca_err != ESP_OK)
    {
        ESP_LOGI(STA_LOG_TAG, "set global CA store NOK !\n");
        return;
    }

    ESP_LOGI(STA_LOG_TAG, "Global CA store initialized !\n");
}

// Function to make HTTP GET request
void make_http_request() {

    //request
    esp_http_client_config_t config = {
        .url = "https://www.google.com",
        .use_global_ca_store = true,
        .event_handler = http_event_handler,
    };

    ESP_LOGI(STA_LOG_TAG, "to esp_http_client_init");
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Perform the GET request
    ESP_LOGI(STA_LOG_TAG, "to http client perform");
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(STA_LOG_TAG, "HTTP GET Status Code: %d", status_code);
    } else {
        ESP_LOGE(STA_LOG_TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    ESP_LOGI(STA_LOG_TAG, "to cleanup");
    // Cleanup
    esp_http_client_cleanup(client);
}


void start_trafic(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI(STA_LOG_TAG, "start trafic event");
    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(STA_LOG_TAG, "creat task");
        // xTaskCreatePinnedToCore(tcp_send, "send_tcp_hello", 4096, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(make_http_request, "http_request", 4096, NULL, 1, NULL, 0);
    }
}

void app_main()
{

    vTaskDelay(4000);

    init_nvs();
    init_global_ca_store();

    printf("start sta: \n");

    xEventGroupCreate();
    
    // setup sta
    init_station();

    // wait for IP || put the code below in a task that starts once an IP is aquired
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &start_trafic, NULL);
}