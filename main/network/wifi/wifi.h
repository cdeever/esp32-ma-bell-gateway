#pragma once

#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "../storage/storage.h"

// Maximum length for SSID and password
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64

// WiFi connection timeout in seconds
#define WIFI_CONNECT_TIMEOUT 30

// WiFi event group bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Default WiFi credentials
#define DEFAULT_WIFI_SSID "YOURSSID"
#define DEFAULT_WIFI_PASS "passwd"


/**
 * @brief Initialize WiFi in station mode
 * 
 * @param wifi_event_group Event group for WiFi events
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t wifi_init_sta(EventGroupHandle_t wifi_event_group);

/**
 * @brief Set WiFi credentials
 * 
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t wifi_set_credentials(const char* ssid, const char* password);

/**
 * @brief Get stored WiFi credentials
 * 
 * @param ssid Buffer to store SSID
 * @param ssid_len Length of SSID buffer
 * @param password Buffer to store password
 * @param pass_len Length of password buffer
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t wifi_get_credentials(char* ssid, size_t ssid_len, char* password, size_t pass_len);

/**
 * @brief Connect to WiFi using stored credentials
 * 
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t wifi_connect(void);

/**
 * @brief Disconnect from WiFi
 * 
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t wifi_disconnect(void);