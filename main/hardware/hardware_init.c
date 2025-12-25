/*
 * Hardware Subsystem Initialization
 */

#include "hardware_init.h"
#include "gpio_pcm_config.h"
#include "esp_log.h"

static const char *TAG = "HW_INIT";

esp_err_t hardware_init(void)
{
    ESP_LOGI(TAG, "Initializing hardware peripherals");

    // Initialize GPIO and PCM configuration
    app_gpio_pcm_io_cfg();

    ESP_LOGI(TAG, "Hardware peripherals initialized successfully");
    return ESP_OK;
}
