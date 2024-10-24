
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/icmp.h"
#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_wifi.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



void send_ping_to_host(const char *target_ip) {
    printf("send_ping_to_host\n");
    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        ESP_LOGE("PING", "Failed to create socket");
        return;
    }

    printf("memset(&addr, 0, sizeof(addr));\n");
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(target_ip);

    char packet[64];
    struct icmp_echo_hdr *icmp_hdr = (struct icmp_echo_hdr *)packet;
    icmp_hdr->type = ICMP_ECHO;
    icmp_hdr->code = 0;
    icmp_hdr->id = htons(1);
    icmp_hdr->seqno = htons(1);
    icmp_hdr->chksum = 0;
    icmp_hdr->chksum = inet_chksum(packet, sizeof(packet));

    printf("sendto : -> \n");
    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        ESP_LOGE("PING", "Failed to send ping request");
    } else {
        ESP_LOGI("PING", "Ping request sent to %s", target_ip);
    }

    // Receive the pong response
    char recv_packet[64];
    socklen_t addr_len = sizeof(addr);
    int received_bytes = recvfrom(sock, recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&addr, &addr_len);
    if (received_bytes < 0) {
        ESP_LOGE("PING", "Failed to receive pong response");
    } else {
        struct icmp_echo_hdr *recv_icmp_hdr = (struct icmp_echo_hdr *)recv_packet;
        if (recv_icmp_hdr->type == ICMP_ER && recv_icmp_hdr->code == 0) {
            ESP_LOGI("PING", "Pong received from %s", inet_ntoa(addr.sin_addr));
        } else {
            ESP_LOGW("PING", "Received non-pong ICMP packet");
        }
    }

    close(sock);
}