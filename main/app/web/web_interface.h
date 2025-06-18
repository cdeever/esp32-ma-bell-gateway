#ifndef __WEB_INTERFACE_H__
#define __WEB_INTERFACE_H__

#include <esp_http_server.h>
#include "esp_err.h"

/**
 * @brief Initialize the web interface
 * 
 * @return esp_err_t ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t web_interface_init(void);

/**
 * @brief Stop the web interface
 * 
 * @return esp_err_t ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t web_interface_stop(void);

#endif /* __WEB_INTERFACE_H__ */ 