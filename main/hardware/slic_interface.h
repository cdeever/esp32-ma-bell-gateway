#ifndef __SLIC_INTERFACE_H__
#define __SLIC_INTERFACE_H__

#include "esp_err.h"

/**
 * @brief Initialize SLIC interface monitoring and control
 *
 * Configures GPIO pins for interfacing with HC-5504B SLIC chip:
 * - GPIO 32 (SHD): Off-hook detection input
 * - GPIO 33 (RD): Ring detection input (future)
 * - GPIO 13 (RC): Ring command output (future)
 *
 * Starts monitoring task for SLIC status pins with debouncing.
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t slic_interface_init(void);

#endif /* __SLIC_INTERFACE_H__ */
