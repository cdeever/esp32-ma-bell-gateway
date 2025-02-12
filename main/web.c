#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_http_server.h>
#include <esp_log.h>
#include <string.h>
#include <inttypes.h>

static const char *TAG = "WEB_SERVER";

// Simulated status variables (replace with actual values)
static bool hook_state = false;     // On-hook / Off-hook
static bool ringer_active = false;  // Ringing / Silent
static bool bt_connected = false;   // Bluetooth connected / Disconnected

// Serve the lightweight HTML page
static esp_err_t html_handler(httpd_req_t *req) {
    const char *html_content =
        "<!DOCTYPE html>"
        "<html lang='en'>"
        "<head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>Ma Bell Status</title>"
        "<style>body { font-family: Arial, sans-serif; text-align: center; }</style>"
        "<script>"
        "function updateStatus() {"
        "  fetch('/status').then(response => response.json()).then(data => {"
        "    document.getElementById('hook').innerText = data.hook;"
        "    document.getElementById('ringer').innerText = data.ringer;"
        "    document.getElementById('bluetooth').innerText = data.bluetooth;"
        "  });"
        "}"
        "setInterval(updateStatus, 2000);"
        "window.onload = updateStatus;"
        "</script>"
        "</head>"
        "<body>"
        "<h1>Ma Bell Status</h1>"
        "<p>Hook State: <strong id='hook'>Loading...</strong></p>"
        "<p>Ringer: <strong id='ringer'>Loading...</strong></p>"
        "<p>Bluetooth: <strong id='bluetooth'>Loading...</strong></p>"
        "</body>"
        "</html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_content, strlen(html_content));
    return ESP_OK;
}

// Serve JSON status
static esp_err_t status_handler(httpd_req_t *req) {
    char response[128];
    snprintf(response, sizeof(response),
             "{ \"hook\": \"%s\", \"ringer\": \"%s\", \"bluetooth\": \"%s\" }",
             hook_state ? "Off-hook" : "On-hook",
             ringer_active ? "Ringing" : "Silent",
             bt_connected ? "Connected" : "Disconnected");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}



// Serve JSON with task information
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

// Register the new endpoint in `start_webserver()`
void start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_html = { .uri = "/", .method = HTTP_GET, .handler = html_handler };
        httpd_uri_t uri_status = { .uri = "/status", .method = HTTP_GET, .handler = status_handler };
        httpd_uri_t uri_tasks = { .uri = "/tasks", .method = HTTP_GET, .handler = tasks_handler };  // New task handler

        httpd_register_uri_handler(server, &uri_html);
        httpd_register_uri_handler(server, &uri_status);
        httpd_register_uri_handler(server, &uri_tasks);  // Register task endpoint

        ESP_LOGI(TAG, "Web server started with /tasks endpoint.");
    } else {
        ESP_LOGE(TAG, "Failed to start web server!");
    }
}
