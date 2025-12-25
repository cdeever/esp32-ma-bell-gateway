#ifndef __HARDWARE_INIT_H__
#define __HARDWARE_INIT_H__

#include "esp_err.h"

/**
 * @brief Initialize all hardware peripherals
 *
 * This function:
 * - Initializes GPIO pins
 * - Configures PCM interface
 * - Sets up I2S for audio
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t hardware_init(void);

#endif /* __HARDWARE_INIT_H__ */
