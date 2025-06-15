#ifndef __WEB_INTERFACE_H__
#define __WEB_INTERFACE_H__

#include <esp_http_server.h>

/**
 * @brief Initialize and start the web server
 * 
 * This function initializes the HTTP server and registers all necessary
 * URI handlers for the web interface.
 * 
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t web_interface_init(void);

#endif /* __WEB_INTERFACE_H__ */ 