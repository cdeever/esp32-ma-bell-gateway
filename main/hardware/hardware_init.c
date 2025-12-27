/*
 * Hardware Subsystem Initialization
 */

#include "hardware_init.h"
#include "gpio_pcm_config.h"
#include "phone_hardware.h"
#include "esp_log.h"

static const char *TAG = "HW_INIT";

esp_err_t hardware_init(void)
{
    ESP_LOGI(TAG, "Initializing hardware peripherals");

    // Initialize GPIO and PCM configuration
    app_gpio_pcm_io_cfg();

    // Initialize phone hardware monitoring (off-hook detection)
    ESP_ERROR_CHECK(phone_hardware_init());

    ESP_LOGI(TAG, "Hardware peripherals initialized successfully");
    return ESP_OK;
}
