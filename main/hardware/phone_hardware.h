#ifndef __PHONE_HARDWARE_H__
#define __PHONE_HARDWARE_H__

#include "esp_err.h"

/**
 * @brief Initialize phone hardware monitoring
 *
 * Configures GPIO for off-hook detection and starts monitoring task.
 * Monitors SLIC SHD pin (GPIO 32) for phone off-hook state.
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t phone_hardware_init(void);

#endif /* __PHONE_HARDWARE_H__ */
