#include "web_interface.h"
#include "app/state/ma_bell_state.h"
#include "config/web_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <string.h>
#include <inttypes.h>
#include <esp_http_server.h>

static const char *TAG = "WEB_IF";

// Server handle
static httpd_handle_t server = NULL;
static bool server_running = false;
static SemaphoreHandle_t server_mutex = NULL;

// HTML content for the main page
static const char *html_content =
    "<!DOCTYPE html>"
    "<html lang='en'>"
    "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>Ma Bell Status</title>"
        "<style>"
            "body { font-family: Arial, sans-serif; text-align: center; margin: 20px; }"
            ".status-box { border: 1px solid #ccc; padding: 10px; margin: 10px; display: inline-block; }"
            ".connected { color: green; }"
            ".disconnected { color: red; }"
            ".warning { color: orange; }"
            ".status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px; }"
            ".status-item { padding: 5px; border: 1px solid #eee; }"
        "</style>"
        "<script>"
            "function updateStatus() {"
            "  fetch('/status').then(response => response.json()).then(data => {"
            "    // Phone status"
            "    document.getElementById('hook').innerText = data.phone.hook;"
            "    document.getElementById('hook').className = data.phone.hook === 'Off-hook' ? 'connected' : 'disconnected';"
            "    document.getElementById('ringer').innerText = data.phone.ringer;"
            "    document.getElementById('ringer').className = data.phone.ringer === 'Ringing' ? 'connected' : 'disconnected';"
            "    document.getElementById('dialing').innerText = data.phone.dialing;"
            "    document.getElementById('dialing').className = data.phone.dialing === 'Dialing' ? 'connected' : 'disconnected';"
            "    document.getElementById('last-digit').innerText = data.phone.last_digit;"
            ""
            "    // Bluetooth status"
            "    document.getElementById('bt-conn').innerText = data.bluetooth.connected;"
            "    document.getElementById('bt-conn').className = data.bluetooth.connected === 'Connected' ? 'connected' : 'disconnected';"
            "    document.getElementById('bt-call').innerText = data.bluetooth.in_call;"
            "    document.getElementById('bt-call').className = data.bluetooth.in_call === 'In Call' ? 'connected' : 'disconnected';"
            "    document.getElementById('bt-device').innerText = data.bluetooth.device || 'None';"
            "    document.getElementById('bt-volume').innerText = data.bluetooth.volume;"
            "    document.getElementById('bt-battery').innerText = data.bluetooth.battery;"
            ""
            "    // Network status"
            "    document.getElementById('wifi').innerText = data.network.wifi;"
            "    document.getElementById('wifi').className = data.network.wifi === 'Connected' ? 'connected' : 'disconnected';"
            "    document.getElementById('ip').innerText = data.network.ip || 'None';"
            "    document.getElementById('rssi').innerText = data.network.rssi + ' dBm';"
            ""
            "    // System status"
            "    document.getElementById('sys-battery').innerText = data.system.battery + '%';"
            "    document.getElementById('sys-battery').className = data.system.battery < 20 ? 'warning' : 'connected';"
            "    document.getElementById('sys-temp').innerText = data.system.temperature + '°C';"
            "    document.getElementById('sys-temp').className = data.system.temperature > 80 ? 'warning' : 'connected';"
            "  });"
            "}"
            "setInterval(updateStatus, 2000);"
            "window.onload = updateStatus;"
        "</script>"
    "</head>"
    "<body>"
        "<h1>Ma Bell Status</h1>"
        "<div class='status-grid'>"
            "<div class='status-box'>"
                "<h2>Phone Status</h2>"
                "<div class='status-item'>Hook: <strong id='hook'>Loading...</strong></div>"
                "<div class='status-item'>Ringer: <strong id='ringer'>Loading...</strong></div>"
                "<div class='status-item'>Dialing: <strong id='dialing'>Loading...</strong></div>"
                "<div class='status-item'>Last Digit: <strong id='last-digit'>Loading...</strong></div>"
            "</div>"
            "<div class='status-box'>"
                "<h2>Bluetooth Status</h2>"
                "<div class='status-item'>Connection: <strong id='bt-conn'>Loading...</strong></div>"
                "<div class='status-item'>Call Status: <strong id='bt-call'>Loading...</strong></div>"
                "<div class='status-item'>Device: <strong id='bt-device'>Loading...</strong></div>"
                "<div class='status-item'>Volume: <strong id='bt-volume'>Loading...</strong></div>"
                "<div class='status-item'>Battery: <strong id='bt-battery'>Loading...</strong></div>"
            "</div>"
            "<div class='status-box'>"
                "<h2>Network Status</h2>"
                "<div class='status-item'>WiFi: <strong id='wifi'>Loading...</strong></div>"
                "<div class='status-item'>IP: <strong id='ip'>Loading...</strong></div>"
                "<div class='status-item'>Signal: <strong id='rssi'>Loading...</strong></div>"
            "</div>"
            "<div class='status-box'>"
                "<h2>System Status</h2>"
                "<div class='status-item'>Battery: <strong id='sys-battery'>Loading...</strong></div>"
                "<div class='status-item'>Temperature: <strong id='sys-temp'>Loading...</strong></div>"
            "</div>"
        "</div>"
    "</body>"
    "</html>";

// Error handler for the web server
static esp_err_t http_error_handler(httpd_req_t *req, httpd_err_code_t err) {
    ESP_LOGE(TAG, "HTTP error %d occurred", err);
    
    // Send error response
    char error_msg[100];
    snprintf(error_msg, sizeof(error_msg), "{\"error\":\"HTTP error %d\"}", err);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_status(req, "500 Internal Server Error");
    httpd_resp_send(req, error_msg, strlen(error_msg));
    return ESP_OK;
}

// Handler for the main HTML page
static esp_err_t html_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received request for HTML page");
    
    if (!server_running) {
        ESP_LOGE(TAG, "Server not running when HTML request received");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server not running");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Expires", "0");
    
    esp_err_t ret = httpd_resp_send(req, html_content, strlen(html_content));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send HTML response");
    } else {
        ESP_LOGI(TAG, "Successfully sent HTML response");
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
    char response[512];
    
    snprintf(response, sizeof(response),
             "{"
             "  \"phone\": {"
             "    \"hook\": \"%s\","
             "    \"ringer\": \"%s\","
             "    \"dialing\": \"%s\","
             "    \"last_digit\": \"%s\""
             "  },"
             "  \"bluetooth\": {"
             "    \"connected\": \"%s\","
             "    \"in_call\": \"%s\","
             "    \"device\": \"%s\","
             "    \"volume\": \"%d\","
             "    \"battery\": \"%d\""
             "  },"
             "  \"network\": {"
             "    \"wifi\": \"%s\","
             "    \"ip\": \"%s\","
             "    \"rssi\": %d"
             "  },"
             "  \"system\": {"
             "    \"battery\": %d,"
             "    \"temperature\": %d"
             "  }"
             "}",
             // Phone status
             ma_bell_state_phone_bits_set(PHONE_STATE_OFF_HOOK) ? "Off-hook" : "On-hook",
             ma_bell_state_phone_bits_set(PHONE_STATE_RINGING) ? "Ringing" : "Silent",
             ma_bell_state_phone_bits_set(PHONE_STATE_DIALING) ? "Dialing" : "Idle",
             state->phone.last_digit == INVALID_DIGIT ? "None" : (char[]){state->phone.last_digit + '0', '\0'},
             // Bluetooth status
             ma_bell_state_bluetooth_bits_set(BT_STATE_CONNECTED) ? "Connected" : "Disconnected",
             ma_bell_state_bluetooth_bits_set(BT_STATE_IN_CALL) ? "In Call" : "Idle",
             state->bluetooth.device_name,
             state->bluetooth.volume,
             state->bluetooth.battery_level * 20, // Convert 0-5 to 0-100
             // Network status
             ma_bell_state_network_bits_set(NET_STATE_WIFI_CONNECTED) ? "Connected" : "Disconnected",
             state->network.ip_address,
             state->network.rssi,
             // System status
             state->system.battery_level,
             state->system.temperature);

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