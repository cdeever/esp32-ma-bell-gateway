#include "slic_interface.h"
#include "config/pin_assignments.h"
#include "app/state/ma_bell_state.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "slic_if";

// Debounce timing
#define HOOK_DEBOUNCE_MS 50
#define POLL_INTERVAL_MS 20

// Task handle
static TaskHandle_t slic_monitor_task_handle = NULL;

// Debounce state
static bool last_hook_state = true;  // true = on-hook (default)
static TickType_t last_hook_change_time = 0;

/**
 * @brief Task to monitor SLIC interface status
 *
 * Polls SLIC input pins for status changes:
 * - GPIO 32 (SHD pin): Off-hook detection
 * The SLIC SHD pin goes LOW when the phone is off-hook.
 */
static void slic_monitor_task(void *arg)
{
    ESP_LOGI(TAG, "SLIC monitor task started");

    while (1) {
        // Read hook state - SHD pin is active LOW (0 = off-hook, 1 = on-hook)
        int gpio_level = gpio_get_level(PIN_OFF_HOOK_DETECT);
        bool current_hook_state = (gpio_level == 1);  // true = on-hook

        // Check for state change with debouncing
        if (current_hook_state != last_hook_state) {
            TickType_t now = xTaskGetTickCount();
            TickType_t elapsed_ms = pdTICKS_TO_MS(now - last_hook_change_time);

            if (elapsed_ms > HOOK_DEBOUNCE_MS) {
                // State change confirmed after debounce period
                last_hook_state = current_hook_state;
                last_hook_change_time = now;

                if (current_hook_state) {
                    // On-hook (handset replaced)
                    ESP_LOGI(TAG, "Phone on-hook detected");
                    ma_bell_state_update_phone_bits(0, PHONE_STATE_OFF_HOOK);
                } else {
                    // Off-hook (handset lifted)
                    ESP_LOGI(TAG, "Phone off-hook detected");
                    ma_bell_state_update_phone_bits(PHONE_STATE_OFF_HOOK, 0);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }
}

esp_err_t slic_interface_init(void)
{
    ESP_LOGI(TAG, "Initializing SLIC interface monitoring");

    // Configure off-hook detect pin (GPIO 32) as input with pull-up
    // Pull-up ensures pin reads high (on-hook) when SLIC not connected
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_OFF_HOOK_DETECT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE  // Using polling, no interrupts
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure off-hook detect GPIO: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "GPIO %d configured for off-hook detection", PIN_OFF_HOOK_DETECT);

    // Read initial state
    int initial_level = gpio_get_level(PIN_OFF_HOOK_DETECT);
    last_hook_state = (initial_level == 1);
    last_hook_change_time = xTaskGetTickCount();

    ESP_LOGI(TAG, "Initial hook state: %s", last_hook_state ? "on-hook" : "off-hook");

    // Create monitoring task
    BaseType_t task_ret = xTaskCreate(
        slic_monitor_task,
        "slic_monitor",
        2048,           // Stack size
        NULL,           // Parameters
        5,              // Priority
        &slic_monitor_task_handle
    );

    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create SLIC monitor task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "SLIC interface monitoring started successfully");
    return ESP_OK;
}
