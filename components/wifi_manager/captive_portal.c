/**
 * @file captive_portal.c
 * @brief Captive Portal HTTP Server Implementation
 */

#include "captive_portal.h"
#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include <string.h>

static const char *TAG = "captive_portal";
static httpd_handle_t server = NULL;

// HTML content for the configuration page
static const char *html_page_header = 
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>SmartLove WiFi Setup</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#f0f0f0}"
    ".container{max-width:500px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}"
    "h1{color:#333;text-align:center;margin-bottom:30px}"
    ".form-group{margin-bottom:20px}"
    "label{display:block;margin-bottom:5px;color:#555;font-weight:bold}"
    "input,select{width:100%;padding:10px;border:1px solid #ddd;border-radius:5px;box-sizing:border-box;font-size:14px}"
    "button{width:100%;padding:12px;background:#007bff;color:white;border:none;border-radius:5px;cursor:pointer;font-size:16px;font-weight:bold}"
    "button:hover{background:#0056b3}"
    ".scan-btn{background:#28a745;margin-bottom:10px}"
    ".scan-btn:hover{background:#1e7e34}"
    ".network-list{max-height:200px;overflow-y:auto;border:1px solid #ddd;border-radius:5px;margin-bottom:10px}"
    ".network-item{padding:10px;cursor:pointer;border-bottom:1px solid #eee}"
    ".network-item:hover{background:#f5f5f5}"
    ".network-item:last-child{border-bottom:none}"
    ".signal{float:right;color:#888}"
    ".lock{color:#ff9800}"
    ".status{text-align:center;padding:10px;margin-top:15px;border-radius:5px;display:none}"
    ".status.success{background:#d4edda;color:#155724;border:1px solid #c3e6cb}"
    ".status.error{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb}"
    ".footer{text-align:center;margin-top:20px;color:#888;font-size:12px}"
    "</style></head><body>"
    "<div class='container'>"
    "<h1>🔧 SmartLove WiFi Setup</h1>";

static const char *html_page_footer = 
    "<div class='footer'>SmartLove - WiFi Configuration</div>"
    "</div></body></html>";

/**
 * @brief Generate network list HTML
 */
static esp_err_t generate_network_list(char *buffer, size_t max_len)
{
    wifi_ap_record_t ap_records[20];
    uint16_t ap_count = 20;
    
    if (wifi_manager_get_scan_results(ap_records, ap_count, &ap_count) != ESP_OK) {
        snprintf(buffer, max_len, "<p>No networks found. Click scan to search.</p>");
        return ESP_OK;
    }

    size_t offset = 0;
    offset += snprintf(buffer + offset, max_len - offset, "<div class='network-list'>");
    
    for (int i = 0; i < ap_count && offset < max_len - 200; i++) {
        const char *security = (ap_records[i].authmode == WIFI_AUTH_OPEN) ? "" : " 🔒";
        int8_t rssi = ap_records[i].rssi;
        const char *signal = (rssi > -50) ? "●●●●" : (rssi > -60) ? "●●●○" : (rssi > -70) ? "●●○○" : "●○○○";
        
        offset += snprintf(buffer + offset, max_len - offset,
            "<div class='network-item' onclick='document.getElementById(\"ssid\").value=\"%s\"'>"
            "%s%s <span class='signal'>%s</span></div>",
            (const char *)ap_records[i].ssid, (const char *)ap_records[i].ssid, security, signal);
    }
    
    offset += snprintf(buffer + offset, max_len - offset, "</div>");
    return ESP_OK;
}

/**
 * @brief Handler for root page
 */
static esp_err_t root_handler(httpd_req_t *req)
{
    char network_list[2048] = {0};
    generate_network_list(network_list, sizeof(network_list));
    
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr_chunk(req, html_page_header);
    
    httpd_resp_sendstr_chunk(req, 
        "<button class='scan-btn' onclick='window.location.href=\"/scan\"'>🔍 Scan Networks</button>");
    
    httpd_resp_sendstr_chunk(req, network_list);
    
    httpd_resp_sendstr_chunk(req,
        "<form method='POST' action='/connect' onsubmit='showStatus()'>"
        "<div class='form-group'>"
        "<label for='ssid'>WiFi Network (SSID)</label>"
        "<input type='text' id='ssid' name='ssid' required placeholder='Enter WiFi name'>"
        "</div>"
        "<div class='form-group'>"
        "<label for='password'>Password</label>"
        "<input type='password' id='password' name='password' placeholder='Enter password (leave empty for open networks)'>"
        "</div>"
        "<button type='submit'>Connect to WiFi</button>"
        "</form>"
        "<div class='status' id='status'></div>"
        "<script>"
        "function showStatus(){document.getElementById('status').style.display='block';"
        "document.getElementById('status').className='status';document.getElementById('status').textContent='Connecting...';}"
        "</script>");
    
    httpd_resp_sendstr_chunk(req, html_page_footer);
    httpd_resp_sendstr_chunk(req, NULL);
    
    return ESP_OK;
}

/**
 * @brief Handler for scan request
 */
static esp_err_t scan_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Starting WiFi scan...");
    wifi_manager_scan();
    
    // Wait for scan to complete
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Redirect back to root
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_sendstr(req, "Redirecting...");
    
    return ESP_OK;
}

/**
 * @brief Parse URL encoded form data
 */
static esp_err_t parse_form_data(const char *data, const char *key, char *value, size_t value_size)
{
    char search_key[64];
    snprintf(search_key, sizeof(search_key), "%s=", key);
    
    const char *start = strstr(data, search_key);
    if (!start) {
        return ESP_ERR_NOT_FOUND;
    }
    
    start += strlen(search_key);
    const char *end = strchr(start, '&');
    size_t len = end ? (end - start) : strlen(start);
    
    if (len >= value_size) {
        len = value_size - 1;
    }
    
    // URL decode
    size_t j = 0;
    for (size_t i = 0; i < len && j < value_size - 1; i++) {
        if (start[i] == '+') {
            value[j++] = ' ';
        } else if (start[i] == '%' && i + 2 < len) {
            char hex[3] = {start[i+1], start[i+2], 0};
            value[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else {
            value[j++] = start[i];
        }
    }
    value[j] = '\0';
    
    return ESP_OK;
}

/**
 * @brief Handler for connect request
 */
static esp_err_t connect_handler(httpd_req_t *req)
{
    char content[256];
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);
    
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to receive data");
        return ESP_FAIL;
    }
    
    content[ret] = '\0';
    
    char ssid[33] = {0};
    char password[65] = {0};
    
    if (parse_form_data(content, "ssid", ssid, sizeof(ssid)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "SSID missing");
        return ESP_FAIL;
    }
    
    parse_form_data(content, "password", password, sizeof(password));
    
    ESP_LOGI(TAG, "Attempting to connect to: %s", ssid);
    
    // Save credentials and attempt connection
    esp_err_t err = wifi_manager_connect(ssid, password, true);
    
    // Send response
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr_chunk(req, html_page_header);
    
    if (err == ESP_OK) {
        httpd_resp_sendstr_chunk(req,
            "<div class='status success'>"
            "✅ Connecting to WiFi...<br>"
            "SmartLove will restart and connect to your network.<br>"
            "You can close this page."
            "</div>"
            "<script>setTimeout(function(){window.location.href='/'},3000);</script>");
    } else {
        httpd_resp_sendstr_chunk(req,
            "<div class='status error'>"
            "❌ Failed to connect. Please check credentials and try again."
            "</div>"
            "<script>setTimeout(function(){window.location.href='/'},3000);</script>");
    }
    
    httpd_resp_sendstr_chunk(req, html_page_footer);
    httpd_resp_sendstr_chunk(req, NULL);
    
    return ESP_OK;
}

/**
 * @brief Handler for captive portal detection (iOS, Android, etc.)
 */
static esp_err_t captive_portal_handler(httpd_req_t *req)
{
    // Redirect to root page
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
    httpd_resp_sendstr(req, "");
    return ESP_OK;
}

esp_err_t captive_portal_start(void)
{
    if (server != NULL) {
        ESP_LOGW(TAG, "Captive portal already running");
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;
    config.lru_purge_enable = true;
    config.stack_size = 8192;

    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return ESP_FAIL;
    }

    // Register handlers
    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &root);

    httpd_uri_t scan = {
        .uri = "/scan",
        .method = HTTP_GET,
        .handler = scan_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &scan);

    httpd_uri_t connect = {
        .uri = "/connect",
        .method = HTTP_POST,
        .handler = connect_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &connect);

    // Captive portal detection endpoints
    const char *captive_endpoints[] = {
        "/generate_204",       // Android
        "/hotspot-detect.html", // iOS
        "/connecttest.txt",    // Windows
        "/success.txt",        // Firefox
        NULL
    };

    for (int i = 0; captive_endpoints[i] != NULL; i++) {
        httpd_uri_t captive = {
            .uri = captive_endpoints[i],
            .method = HTTP_GET,
            .handler = captive_portal_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &captive);
    }

    ESP_LOGI(TAG, "Captive portal started on http://192.168.4.1");
    return ESP_OK;
}

esp_err_t captive_portal_stop(void)
{
    if (server == NULL) {
        return ESP_OK;
    }

    httpd_stop(server);
    server = NULL;
    
    ESP_LOGI(TAG, "Captive portal stopped");
    return ESP_OK;
}

bool captive_portal_is_running(void)
{
    return (server != NULL);
}
