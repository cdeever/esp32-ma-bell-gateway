#ifndef __BT_INIT_H__
#define __BT_INIT_H__

#include "esp_err.h"

/**
 * @brief Initialize the Bluetooth subsystem
 *
 * This function:
 * - Initializes Bluetooth controller and Bluedroid stack
 * - Starts the BT application task (FIXES bt_app_task_start_up bug)
 * - Sets device name and PIN
 * - Initializes HFP client profile
 * - Registers GAP and HFP callbacks
 * - Starts connection manager and reconnection task
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t bluetooth_init(void);

/**
 * @brief Shutdown the Bluetooth subsystem
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t bluetooth_shutdown(void);

#endif /* __BT_INIT_H__ */
