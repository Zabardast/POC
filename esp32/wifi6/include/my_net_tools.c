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
static const char *STA_WIFI = "wifi STA";
static const char *NAPT = "ip_forwarding";

#include <sys/socket.h>
static const char *STA_wireshark = "tcp for wireshark";
#define PORT 4242

void ip_forwarding();
void initialize_ping();

#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"


void wifi_sta_lost_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    printf("ip forwarding event\n");

    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        // reconnect
        printf("esp_wifi_connect : %i\n",esp_wifi_connect());
    }
}


void ip_forwarding(struct esp_ip4_addr p_addr) // or nat ???
{
    printf("ip_forwarding\n");

    // debug ?? TODO: remove this
    // heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

    ESP_LOGI(NAPT, "enable ip_napt for address " IPSTR, IP2STR(&p_addr));
    // printf("ip address: %i\n", (int)p_addr.addr);

    // Enable NAPT on the STA IP address
    ip_napt_enable(p_addr.addr, 1);

    ESP_LOGI(NAPT, "ip_napt_enabled for address " IPSTR, IP2STR(&p_addr));
    
}

void wifi_sta_ap_event_ip_forwarding(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    printf("ip forwarding event\n");

    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        printf("event IP_EVENT_STA_GOT_IP\n");

        ip_event_got_ip_t* ip_event_data = (ip_event_got_ip_t*) event_data;

        printf("frw ip : \n");
        ip_forwarding(ip_event_data->ip_info.ip);

        // printf("frw gw : \n");
        // ip_forwarding(ip_event_data->ip_info.gw);
        // initialize_ping();
    }
}

void init_AP_STA()
{
    // first the sta to connect to provider AP
    esp_netif_init();

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    // esp_netif_napt_enable(sta_netif);
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

    esp_err_t sta_connect = esp_wifi_connect();
    if(sta_connect == ESP_OK)
    {
        ESP_LOGI(STA_WIFI, "Wifi sta connected successfully\n");
    }else{
        ESP_LOGI(STA_WIFI, "Wifi sta failed to connect\n");
    }

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
// TODO: remove the following 
    esp_err_t enab_nat_err = esp_netif_napt_enable(ap_netif);
    if (enab_nat_err == ESP_OK) {
        ESP_LOGI("NAPT", "NAPT successfully enabled");
    } else {
        ESP_LOGE("NAPT", "Failed to enable NAPT: %s", esp_err_to_name(enab_nat_err));
    }

// ESP32 STATION

    // int addr_family = 0;
    // int ip_protocol = 0;
    // char rx_buffer[128];
    // char host_ip[] = "10.0.0.218";

    // struct sockaddr_in dest_addr;
    // inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
    // dest_addr.sin_family = AF_INET;
    // dest_addr.sin_port = htons(PORT);
    // addr_family = AF_INET;
    // ip_protocol = IPPROTO_IP;

    char *payload = "Message from ESP32 ";

    printf("entering the for(;;) loop\n");

    int sock = socket(AF_INET, SOCK_RAW, htons(0x806));
    if (sock < 0) {
        perror("socket");
        // return 1;
    }

    struct sockaddr_in src_addr;
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(0);
    inet_aton("10.0.0.218", &src_addr.sin_addr);

    //get mac
    // struct sockaddr_ll src_mac;
    // src_mac.sll_family = AF_LINK;
    // src_mac.sll_protocol = htons(0x0806);
    // Set your own MAC address here

    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);

    struct arp_header_t {
        uint16_t htype;
        uint16_t hlen;
        uint8_t opcode;
        uint8_t sender_ip[4];
        uint8_t sender_mac[6];
        uint8_t target_ip[4];
        uint8_t target_mac[6];
    }arp_header;

    // arp_header arp;
    // arp_header.htype = htons(1); // Ethernet
    // arp_header.hlen = htons(6); // Ethernet header length
    // arp_header.opcode = TWT_REQUEST;
    // memcpy(arp_header.sender_ip, "10.0.0.153", 4);
    // memcpy(arp_header.sender_mac, mac, 6);
    // memcpy(arp_header.target_ip, ARP_TARGET_IP, 4);
    // memcpy(arp_header.target_mac, ARP_TARGET_MAC, 6);

    // char* packet = (char*)&arp;
    // int packet_len = sizeof(arp);

    // sendto(sock, packet, packet_len, 0, (struct sockaddr*)&src_addr, sizeof(src_addr));

    // close(sock);

}


////////////////////////////////////////////////////////////////

#include "ping/ping_sock.h"
#include <string.h>
// #include "sdkconfig.h"
#include "esp_console.h"

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_event.h"

#include "nvs_flash.h"
#include "argtable3/argtable3.h"
// #include "protocol_examples_common.h"
#include "ping/ping_sock.h"

static void test_on_ping_success(esp_ping_handle_t hdl, void *args)
{
    // optionally, get callback arguments
    // const char* str = (const char*) args;
    // printf("%s\r\n", str); // "foo"
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    printf("%li bytes from %s icmp_seq=%d ttl=%d time=%li ms\n",
           recv_len, inet_ntoa(target_addr.u_addr.ip4), seqno, ttl, elapsed_time);
}

static void test_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    printf("From %s icmp_seq=%d timeout\n", inet_ntoa(target_addr.u_addr.ip4), seqno);
}

static void test_on_ping_end(esp_ping_handle_t hdl, void *args)
{
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;

    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    printf("%li packets transmitted, %li received, time %lims\n", transmitted, received, total_time_ms);
}

void initialize_ping()
{
    /* convert URL to IP address */
    ip_addr_t target_addr;
    struct addrinfo hint;
    struct addrinfo *res = NULL;
    memset(&hint, 0, sizeof(hint));
    memset(&target_addr, 0, sizeof(target_addr));
    // getaddrinfo("www.espressif.com", NULL, &hint, &res);

    getaddrinfo("www.espressif.com", NULL, &hint, &res);
    struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
    inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
    freeaddrinfo(res);

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;          // target IP address
    ping_config.count = ESP_PING_COUNT_INFINITE;    // ping in infinite mode, esp_ping_stop can stop it
    ping_config.interface = 1;

    /* set callback functions */
    esp_ping_callbacks_t cbs;
    cbs.on_ping_success = test_on_ping_success;
    cbs.on_ping_timeout = test_on_ping_timeout;
    cbs.on_ping_end = test_on_ping_end;
    cbs.cb_args = "foo";  // arguments that feeds to all callback functions, can be NULL
    // cbs.cb_args = eth_event_group;

    esp_ping_handle_t ping;
    esp_ping_new_session(&ping_config, &cbs, &ping);

    printf("START PING\n");
    esp_ping_start(ping);
}
