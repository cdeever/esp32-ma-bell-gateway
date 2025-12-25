#ifndef __BT_CONNECTION_MANAGER_H__
#define __BT_CONNECTION_MANAGER_H__

#include "esp_err.h"
#include "esp_gap_bt_api.h"

/**
 * @brief Initialize the Bluetooth connection manager
 *
 * This starts the reconnection task and registers GAP callback
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t bt_connection_manager_init(void);

/**
 * @brief GAP callback for connection management
 *
 * Handles discovery, authentication, and PIN requests
 *
 * @param event GAP event type
 * @param param Event parameters
 */
void bt_connection_manager_gap_cb(esp_bt_gap_cb_event_t event,
                                   esp_bt_gap_cb_param_t *param);

/**
 * @brief Get connection status
 *
 * @return true if connected, false otherwise
 */
bool bt_connection_manager_is_connected(void);

#endif /* __BT_CONNECTION_MANAGER_H__ */
