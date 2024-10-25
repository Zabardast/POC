
// #include "lwip/sockets.h"
// #include "lwip/netdb.h"
// #include "lwip/icmp.h"
// #include "lwip/ip_addr.h"
// #include "lwip/tcpip.h"
// #include "lwip/inet.h"
// #include "lwip/inet_chksum.h"
// #include "lwip/err.h"
// #include "lwip/sys.h"

#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"


#include "lwip/init.h"
#include "lwip/ip_addr.h" // Include this header for IP4_ADDR



#include "esp_wifi.h"
#include "esp_log.h"

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

#define DEFAULT_SCAN_LIST_SIZE 10


static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
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
    }
}

void init_station()
{
    //new
// Initialize the network interface
    esp_netif_init();
    esp_event_loop_create_default();

// Set up the Wi-Fi station interface
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();

        // Configure static IP
    // esp_netif_ip_info_t ip_info;
    // IP4_ADDR(&ip_info.ip, 10, 0, 0, 215);  // Set your desired static IP
    // IP4_ADDR(&ip_info.gw, 10, 0, 0, 1);    // Set your router's IP address
    // IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0); // Set your subnet mask
    // esp_netif_set_ip_info(netif, &ip_info);

    //end new

    // initialize wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    printf("esp init: \n");
    // esp_wifi_init uses NVS in osi_nvs_open() function which seems to be a custom function not meant for users.
    // I can only speculate that esp_wifi_init uses nvs to store configuration information.
    esp_wifi_init(&cfg);

    // add event handler for wifi and ip address change
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);


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
            .threshold.authmode = WIFI_AUTH_WPA3_PSK,
            .scan_method = WIFI_FAST_SCAN,
            .bssid_set = false,
            .channel = 6
        },
    };



//bugfix
    wifi_country_t wifi_country = {
        .cc = "US",       // Set your country code (e.g., "US" for the United States)
        .schan = 1,
        .nchan = 13,
        .max_tx_power = 20,
        .policy = WIFI_COUNTRY_POLICY_AUTO
    };
    esp_wifi_set_country(&wifi_country);
    // esp_wifi_set_ant(WIFI_ANT_MODE_AUTO);
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B);
//

    // needs STA to be initialized
    esp_wifi_set_config(ESP_IF_WIFI_STA, &n_wifi_config);
    // starts wifi from the previous settings
    esp_err_t wf_result = esp_wifi_start();

    printf("Wifi start result: %i\n", wf_result);

    // get dB
    esp_wifi_scan_start(NULL, true);
    
    //get results from scan
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE] = {0};
    uint16_t ap_count = 0;
    printf("ap_info : %u\n", sizeof(ap_info));


    esp_wifi_scan_get_ap_records(&number, ap_info);
    esp_wifi_scan_get_ap_num(&ap_count);

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

/*
    // scan networks in the area!!
    printf("start scanning networks : \n");
    esp_wifi_scan_start(NULL, true);

    //get results from scan
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE] = {0};
    uint16_t ap_count = 0;
    printf("ap_info : %u\n", sizeof(ap_info));


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

    if(!good_to_connect)
    {
        printf("AP not found\n");
        return;
    }
*/
    // esp_err_t wf_connection = esp_wifi_connect();
    // printf("Wifi connection result: %s\n", (wf_connection == ESP_OK) ? "good link" : "failed");



}


void send_ping_to_host(const char *target_ip) {
    printf("send_ping_to_host\n");
    // struct sockaddr_in addr;
    //tst
    // sys_mbox_t mbox;
    // sys_mbox_new(&mbox,20);
    
    // tcpip_init_done_fn(mbox);
    // tcpip_init(tcpip_init_done_fn(&mbox), NULL);
    // tcpip_init();

    esp_netif_init();
    
    //endtst



    // int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    // int sock = socket(AF_INET, SOCK_STREAM, 0);
    // if (sock < 0) {
    //     ESP_LOGE("PING", "Failed to create socket");
    //     return;
    // }

    printf("memset(&addr, 0, sizeof(addr));\n");
    // memset(&addr, 0, sizeof(addr));
    // addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = inet_addr(target_ip);

    // char packet[64];
    // struct icmp_echo_hdr *icmp_hdr = (struct icmp_echo_hdr *)packet;
    // icmp_hdr->type = ICMP_ECHO;
    // icmp_hdr->code = 0;
    // icmp_hdr->id = htons(1);
    // icmp_hdr->seqno = htons(1);
    // icmp_hdr->chksum = 0;
    // icmp_hdr->chksum = inet_chksum(packet, sizeof(packet));

    // printf("sendto : -> \n");
    // if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    //     ESP_LOGE("PING", "Failed to send ping request");
    // } else {
    //     ESP_LOGI("PING", "Ping request sent to %s", target_ip);
    // }

    // // Receive the pong response
    // char recv_packet[64];
    // socklen_t addr_len = sizeof(addr);
    // int received_bytes = recvfrom(sock, recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&addr, &addr_len);
    // if (received_bytes < 0) {
    //     ESP_LOGE("PING", "Failed to receive pong response");
    // } else {
    //     struct icmp_echo_hdr *recv_icmp_hdr = (struct icmp_echo_hdr *)recv_packet;
    //     if (recv_icmp_hdr->type == ICMP_ER && recv_icmp_hdr->code == 0) {
    //         ESP_LOGI("PING", "Pong received from %s", inet_ntoa(addr.sin_addr));
    //     } else {
    //         ESP_LOGW("PING", "Received non-pong ICMP packet");
    //     }
    // }

    // close(sock);
}