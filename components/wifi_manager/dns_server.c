/**
 * @file dns_server.c
 * @brief Simple DNS Server for Captive Portal
 * 
 * This DNS server responds to all DNS queries with the AP's IP address,
 * which is necessary for the captive portal to work properly.
 */

#include "dns_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "dns_server";

#define DNS_PORT 53
#define DNS_MAX_PACKET_SIZE 512

static int dns_socket = -1;
static TaskHandle_t dns_task_handle = NULL;
static bool dns_running = false;

/**
 * @brief DNS packet header structure
 */
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;
} __attribute__((packed)) dns_header_t;

/**
 * @brief Parse and respond to DNS query
 */
static void dns_process_request(uint8_t *request, size_t req_len, uint8_t *response, size_t *resp_len, uint32_t ap_ip)
{
    if (req_len < sizeof(dns_header_t)) {
        *resp_len = 0;
        return;
    }

    dns_header_t *req_header = (dns_header_t *)request;
    dns_header_t *resp_header = (dns_header_t *)response;

    // Copy request to response
    memcpy(response, request, req_len);
    
    // Set response flags: Response, Authoritative, No error
    resp_header->flags = htons(0x8180);
    resp_header->an_count = resp_header->qd_count; // Answer count = Question count

    // Append answer for each question
    uint8_t *answer_ptr = response + req_len;
    
    for (int i = 0; i < ntohs(req_header->qd_count); i++) {
        // Answer: Name pointer (compression)
        *answer_ptr++ = 0xC0;
        *answer_ptr++ = 0x0C;
        
        // Type A (IPv4)
        *answer_ptr++ = 0x00;
        *answer_ptr++ = 0x01;
        
        // Class IN
        *answer_ptr++ = 0x00;
        *answer_ptr++ = 0x01;
        
        // TTL (60 seconds)
        *answer_ptr++ = 0x00;
        *answer_ptr++ = 0x00;
        *answer_ptr++ = 0x00;
        *answer_ptr++ = 0x3C;
        
        // Data length (4 bytes for IPv4)
        *answer_ptr++ = 0x00;
        *answer_ptr++ = 0x04;
        
        // IP address
        *answer_ptr++ = (ap_ip >> 0) & 0xFF;
        *answer_ptr++ = (ap_ip >> 8) & 0xFF;
        *answer_ptr++ = (ap_ip >> 16) & 0xFF;
        *answer_ptr++ = (ap_ip >> 24) & 0xFF;
    }

    *resp_len = answer_ptr - response;
}

/**
 * @brief DNS server task
 */
static void dns_server_task(void *pvParameters)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    uint8_t rx_buffer[DNS_MAX_PACKET_SIZE];
    uint8_t tx_buffer[DNS_MAX_PACKET_SIZE];
    
    // Get AP IP address
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(ap_netif, &ip_info);
    uint32_t ap_ip = ip_info.ip.addr;
    
    ESP_LOGI(TAG, "DNS Server running on %d.%d.%d.%d:%d",
             IP2STR(&ip_info.ip), DNS_PORT);

    while (dns_running) {
        int len = recvfrom(dns_socket, rx_buffer, sizeof(rx_buffer), 0,
                          (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (len > 0) {
            size_t resp_len = 0;
            dns_process_request(rx_buffer, len, tx_buffer, &resp_len, ap_ip);
            
            if (resp_len > 0) {
                sendto(dns_socket, tx_buffer, resp_len, 0,
                      (struct sockaddr *)&client_addr, client_addr_len);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

esp_err_t dns_server_start(void)
{
    if (dns_running) {
        ESP_LOGW(TAG, "DNS server already running");
        return ESP_OK;
    }

    // Create UDP socket
    dns_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (dns_socket < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return ESP_FAIL;
    }

    // Bind to DNS port
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(DNS_PORT)
    };

    if (bind(dns_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to bind socket");
        close(dns_socket);
        dns_socket = -1;
        return ESP_FAIL;
    }

    // Set socket to non-blocking
    int flags = fcntl(dns_socket, F_GETFL, 0);
    fcntl(dns_socket, F_SETFL, flags | O_NONBLOCK);

    // Start DNS server task
    dns_running = true;
    if (xTaskCreate(dns_server_task, "dns_server", 4096, NULL, 5, &dns_task_handle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create DNS server task");
        close(dns_socket);
        dns_socket = -1;
        dns_running = false;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "DNS server started");
    return ESP_OK;
}

esp_err_t dns_server_stop(void)
{
    if (!dns_running) {
        return ESP_OK;
    }

    dns_running = false;

    if (dns_task_handle) {
        vTaskDelay(pdMS_TO_TICKS(100)); // Give task time to exit
        dns_task_handle = NULL;
    }

    if (dns_socket >= 0) {
        close(dns_socket);
        dns_socket = -1;
    }

    ESP_LOGI(TAG, "DNS server stopped");
    return ESP_OK;
}

bool dns_server_is_running(void)
{
    return dns_running;
}
