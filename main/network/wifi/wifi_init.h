#ifndef __WIFI_INIT_H__
#define __WIFI_INIT_H__

#include "esp_err.h"

/**
 * @brief Initialize WiFi subsystem and connect to network
 *
 * This function:
 * - Initializes TCP/IP stack
 * - Initializes WiFi driver
 * - Creates event group internally
 * - Connects to WiFi using stored/default credentials
 *
 * @return ESP_OK on success, ESP_FAIL on failure (note: continues without WiFi)
 */
esp_err_t wifi_init_and_connect(void);

#endif /* __WIFI_INIT_H__ */
