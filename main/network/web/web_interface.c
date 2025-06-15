#include "web_interface.h"
#include "state/ma_bell_state.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <string.h>
#include <inttypes.h>

static const char *TAG = "WEB_INTERFACE";

// Server handle
static httpd_handle_t server = NULL;

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

// Handler for the main HTML page
static esp_err_t html_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_content, strlen(html_content));
    return ESP_OK;
}

// Handler for the status JSON endpoint
static esp_err_t status_handler(httpd_req_t *req) {
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

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

// Handler for the tasks JSON endpoint
static esp_err_t tasks_handler(httpd_req_t *req) {
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

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

// Public interface implementation
esp_err_t web_interface_init(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start web server!");
        return ESP_FAIL;
    }

    // Register URI handlers
    httpd_uri_t uri_html = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = html_handler
    };
    httpd_uri_t uri_status = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler
    };
    httpd_uri_t uri_tasks = {
        .uri = "/tasks",
        .method = HTTP_GET,
        .handler = tasks_handler
    };

    httpd_register_uri_handler(server, &uri_html);
    httpd_register_uri_handler(server, &uri_status);
    httpd_register_uri_handler(server, &uri_tasks);

    ESP_LOGI(TAG, "Web server started successfully");
    return ESP_OK;
} 