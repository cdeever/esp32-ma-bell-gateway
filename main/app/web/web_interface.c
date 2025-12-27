#include "web_interface.h"
#include "app/state/ma_bell_state.h"
#include "config/web_config.h"
#include "network/wifi/wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <string.h>
#include <inttypes.h>
#include <esp_http_server.h>

static const char *TAG = "WEB_IF";

// HTTP error codes
#define HTTPD_ERR_RESP_SEND 6  // Client closed connection

// Server handle
static httpd_handle_t server = NULL;
static bool server_running = false;
static SemaphoreHandle_t server_mutex = NULL;

// Cache WiFi SSID to avoid NVS contention during status requests
static char cached_wifi_ssid[33] = "Not configured";

// HTML content for the main page
// JSON API documentation (simple endpoint list)
static const char *api_docs =
    "{"
    "  \"api_version\": \"1.0\","
    "  \"endpoints\": ["
    "    {"
    "      \"path\": \"/\","
    "      \"method\": \"GET\","
    "      \"description\": \"API endpoint list (this page)\""
    "    },"
    "    {"
    "      \"path\": \"/status\","
    "      \"method\": \"GET\","
    "      \"description\": \"System status (phone, bluetooth, wifi, system)\""
    "    },"
    "    {"
    "      \"path\": \"/tasks\","
    "      \"method\": \"GET\","
    "      \"description\": \"FreeRTOS task information\""
    "    }"
    "  ]"
    "}";

// Error handler for the web server
static esp_err_t http_error_handler(httpd_req_t *req, httpd_err_code_t err) {
    // Error code 6 (HTTPD_ERR_RESP_SEND) is benign - client closed connection
    // This happens normally when browser/client finishes receiving response
    if (err == HTTPD_ERR_RESP_SEND) {
        ESP_LOGD(TAG, "Client closed connection (expected behavior)");
        return ESP_OK;
    }

    // Log actual errors
    ESP_LOGE(TAG, "HTTP error %d occurred", err);

    // Send error response
    char error_msg[100];
    snprintf(error_msg, sizeof(error_msg), "{\"error\":\"HTTP error %d\"}", err);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_status(req, "500 Internal Server Error");
    httpd_resp_send(req, error_msg, strlen(error_msg));
    return ESP_OK;
}

// Handler for the API endpoint list
static esp_err_t html_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received request for API endpoint list");

    if (!server_running) {
        ESP_LOGE(TAG, "Server not running when request received");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server not running");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Expires", "0");

    esp_err_t ret = httpd_resp_send(req, api_docs, strlen(api_docs));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send API docs response");
    } else {
        ESP_LOGI(TAG, "Successfully sent API docs response");
    }
    return ret;
}

// Handler for the status JSON endpoint
static esp_err_t status_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received request for status");

    if (!server_running) {
        ESP_LOGE(TAG, "Server not running when status request received");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server not running");
        return ESP_FAIL;
    }

    if (xSemaphoreTake(server_mutex, pdMS_TO_TICKS(WEB_MUTEX_TIMEOUT_MS)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take mutex for status request");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server busy");
        return ESP_FAIL;
    }

    const ma_bell_state_t* state = ma_bell_state_get();

    // Use cached SSID (read once during init to avoid NVS contention)

    // Calculate uptime
    uint32_t uptime_sec = esp_log_timestamp() / 1000;
    uint32_t uptime_hours = uptime_sec / 3600;
    uint32_t uptime_mins = (uptime_sec % 3600) / 60;
    uint32_t uptime_secs = uptime_sec % 60;

    // Streamlined buffer for essential status fields
    char response[800];

    snprintf(response, sizeof(response),
             "{"
             "  \"phone\": {"
             "    \"status\": {"
             "      \"hook\": \"%s\","
             "      \"ringing\": %s,"
             "      \"ring_count\": %d,"
             "      \"dialing\": %s,"
             "      \"last_digit\": \"%s\""
             "    },"
             "    \"tones\": {"
             "      \"dial_tone\": %s,"
             "      \"busy_tone\": %s,"
             "      \"ringback\": %s,"
             "      \"reorder\": %s,"
             "      \"call_waiting\": %s"
             "    }"
             "  },"
             "  \"bluetooth\": {"
             "    \"connected\": %s,"
             "    \"device\": \"%s\","
             "    \"in_call\": %s,"
             "    \"volume\": %d,"
             "    \"phone_battery\": %d,"
             "    \"phone_signal\": %d"
             "  },"
             "  \"wifi\": {"
             "    \"connected\": %s,"
             "    \"ssid\": \"%s\","
             "    \"ip\": \"%s\","
             "    \"rssi\": %d,"
             "    \"channel\": %d"
             "  },"
             "  \"system\": {"
             "    \"uptime\": \"%" PRIu32 "h %" PRIu32 "m %" PRIu32 "s\","
             "    \"error\": %s,"
             "    \"error_code\": %d"
             "  }"
             "}",
             // Phone status
             ma_bell_state_phone_bits_set(PHONE_STATE_OFF_HOOK) ? "Off-hook" : "On-hook",
             ma_bell_state_phone_bits_set(PHONE_STATE_RINGING) ? "true" : "false",
             state->phone.ring_count,
             ma_bell_state_phone_bits_set(PHONE_STATE_DIALING) ? "true" : "false",
             state->phone.last_digit == INVALID_DIGIT ? "None" : (char[]){state->phone.last_digit + '0', '\0'},
             // Phone tones
             ma_bell_state_phone_bits_set(PHONE_STATE_DIAL_TONE) ? "true" : "false",
             ma_bell_state_phone_bits_set(PHONE_STATE_BUSY_TONE) ? "true" : "false",
             ma_bell_state_phone_bits_set(PHONE_STATE_RINGBACK) ? "true" : "false",
             ma_bell_state_phone_bits_set(PHONE_STATE_REORDER_TONE) ? "true" : "false",
             ma_bell_state_phone_bits_set(PHONE_STATE_CALL_WAITING) ? "true" : "false",
             // Bluetooth
             ma_bell_state_bluetooth_bits_set(BT_STATE_CONNECTED) ? "true" : "false",
             state->bluetooth.device_name[0] ? state->bluetooth.device_name : "None",
             ma_bell_state_bluetooth_bits_set(BT_STATE_IN_CALL) ? "true" : "false",
             state->bluetooth.volume,
             state->bluetooth.battery_level * 20,  // Convert 0-5 to 0-100%
             state->bluetooth.signal_strength,
             // WiFi
             ma_bell_state_network_bits_set(NET_STATE_WIFI_CONNECTED) ? "true" : "false",
             cached_wifi_ssid,
             state->network.ip_address[0] ? state->network.ip_address : "0.0.0.0",
             state->network.rssi,
             state->network.channel,
             // System
             uptime_hours, uptime_mins, uptime_secs,
             ma_bell_state_system_bits_set(SYS_STATE_ERROR) ? "true" : "false",
             state->system.error_code);

    xSemaphoreGive(server_mutex);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Expires", "0");

    esp_err_t ret = httpd_resp_send(req, response, strlen(response));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send status response");
    } else {
        ESP_LOGI(TAG, "Successfully sent status response");
    }
    return ret;
}

// Handler for the tasks JSON endpoint
static esp_err_t tasks_handler(httpd_req_t *req) {
    if (!server_running) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server not running");
        return ESP_FAIL;
    }

    if (xSemaphoreTake(server_mutex, pdMS_TO_TICKS(WEB_MUTEX_TIMEOUT_MS)) != pdTRUE) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server busy");
        return ESP_FAIL;
    }

    const int max_tasks = 10;
    TaskStatus_t task_array[max_tasks];
    uint32_t total_runtime;
    uint32_t num_tasks = uxTaskGetSystemState(task_array, max_tasks, &total_runtime);

    char response[512];
    int offset = snprintf(response, sizeof(response), "["); 

    for (int i = 0; i < num_tasks && offset < sizeof(response) - 64; i++) {
        offset += snprintf(response + offset, sizeof(response) - offset,
                         "{\"name\":\"%s\", \"state\":%d, \"priority\":%d, \"stack\":%lu, \"task_no\":%d}%s",
                         task_array[i].pcTaskName, 
                         task_array[i].eCurrentState, 
                         task_array[i].uxCurrentPriority, 
                         (unsigned long)task_array[i].usStackHighWaterMark, 
                         task_array[i].xTaskNumber,
                         (i == num_tasks - 1) ? "" : ", ");
    }

    snprintf(response + offset, sizeof(response) - offset, "]");

    xSemaphoreGive(server_mutex);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Expires", "0");
    
    esp_err_t ret = httpd_resp_send(req, response, strlen(response));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send tasks response");
    }
    return ret;
}

// Public interface implementation
esp_err_t web_interface_init(void) {
    if (server_running) {
        ESP_LOGW(TAG, "Web server already running");
        return ESP_OK;
    }

    // Create mutex for server access
    server_mutex = xSemaphoreCreateMutex();
    if (server_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create server mutex");
        return ESP_FAIL;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = WEB_SERVER_MAX_URI_HANDLERS;
    config.stack_size = WEB_SERVER_STACK_SIZE;
    config.core_id = WEB_SERVER_CORE_ID;
    config.server_port = WEB_SERVER_PORT;
    config.ctrl_port = WEB_SERVER_CTRL_PORT;
    config.send_wait_timeout = WEB_SERVER_SEND_TIMEOUT;
    config.recv_wait_timeout = WEB_SERVER_RECV_TIMEOUT;
    config.lru_purge_enable = true;   // Enable LRU purge
    config.max_resp_headers = WEB_SERVER_MAX_RESP_HEADERS;
    config.backlog_conn = WEB_SERVER_BACKLOG_CONN;
    
    ESP_LOGI(TAG, "Starting web server on port %d with config:", config.server_port);
    ESP_LOGI(TAG, "- Stack size: %d", config.stack_size);
    ESP_LOGI(TAG, "- Max URI handlers: %d", config.max_uri_handlers);
    ESP_LOGI(TAG, "- Max response headers: %d", config.max_resp_headers);
    ESP_LOGI(TAG, "- Backlog connections: %d", config.backlog_conn);
    
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start web server!");
        vSemaphoreDelete(server_mutex);
        return ESP_FAIL;
    }

    // Register URI handlers
    httpd_uri_t uri_html = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = html_handler,
        .user_ctx = NULL
    };
    httpd_uri_t uri_status = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = NULL
    };
    httpd_uri_t uri_tasks = {
        .uri = "/tasks",
        .method = HTTP_GET,
        .handler = tasks_handler,
        .user_ctx = NULL
    };

    // Register error handler
    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_error_handler);
    httpd_register_err_handler(server, HTTPD_500_INTERNAL_SERVER_ERROR, http_error_handler);

    // Register URI handlers
    if (httpd_register_uri_handler(server, &uri_html) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register HTML handler");
        httpd_stop(server);
        vSemaphoreDelete(server_mutex);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Registered HTML handler for /");

    if (httpd_register_uri_handler(server, &uri_status) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register status handler");
        httpd_stop(server);
        vSemaphoreDelete(server_mutex);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Registered status handler for /status");

    if (httpd_register_uri_handler(server, &uri_tasks) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register tasks handler");
        httpd_stop(server);
        vSemaphoreDelete(server_mutex);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Registered tasks handler for /tasks");

    server_running = true;
    ESP_LOGI(TAG, "Web server started successfully on port %d", config.server_port);

    // Cache WiFi SSID once to avoid NVS contention during status requests
    // This happens after WiFi is already connected, so safe to read NVS
    char temp_pass[65];
    esp_err_t ret = wifi_get_credentials(cached_wifi_ssid, sizeof(cached_wifi_ssid),
                                          temp_pass, sizeof(temp_pass));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Cached WiFi SSID: %s", cached_wifi_ssid);
    } else {
        ESP_LOGI(TAG, "No WiFi credentials found in NVS");
        strncpy(cached_wifi_ssid, "Not configured", sizeof(cached_wifi_ssid));
    }

    return ESP_OK;
}

esp_err_t web_interface_stop(void) {
    if (!server_running) {
        return ESP_OK;
    }

    if (xSemaphoreTake(server_mutex, pdMS_TO_TICKS(WEB_MUTEX_TIMEOUT_MS)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take server mutex during shutdown");
        return ESP_FAIL;
    }

    if (httpd_stop(server) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop web server");
        xSemaphoreGive(server_mutex);
        return ESP_FAIL;
    }

    server = NULL;
    server_running = false;
    xSemaphoreGive(server_mutex);
    vSemaphoreDelete(server_mutex);
    server_mutex = NULL;

    ESP_LOGI(TAG, "Web server stopped successfully");
    return ESP_OK;
} 