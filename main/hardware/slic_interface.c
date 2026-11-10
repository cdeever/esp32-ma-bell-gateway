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

// A hook change runs every subscriber's callback on this task (the ringer,
// the line, the event log's formatting), so it needs more than the 2KB a
// bare GPIO poll would
#define SLIC_MONITOR_STACK_SIZE 4096

// Task handle
static TaskHandle_t slic_monitor_task_handle = NULL;

// Debounce state: the accepted hook state, and the raw reading waiting to be accepted
static bool hook_on = true;             // true = on-hook
static bool candidate_on = true;
static TickType_t candidate_since = 0;

static void report_hook(bool on_hook)
{
    if (on_hook) {
        ESP_LOGI(TAG, "Phone on-hook detected");
        ma_bell_state_update_phone_bits(0, PHONE_STATE_OFF_HOOK);
    } else {
        ESP_LOGI(TAG, "Phone off-hook detected");
        ma_bell_state_update_phone_bits(PHONE_STATE_OFF_HOOK, 0);
    }
}

/**
 * @brief Task to monitor SLIC interface status
 *
 * Polls the hook input on GPIO 32: the SLIC's SHD pin, or on the bench a
 * switch to ground. Low is off-hook. A change is accepted once the reading
 * has held for HOOK_DEBOUNCE_MS, so a bouncing contact reports once.
 */
static void slic_monitor_task(void *arg)
{
    ESP_LOGI(TAG, "SLIC monitor task started");

    while (1) {
        bool reading_on = (gpio_get_level(PIN_OFF_HOOK_DETECT) == 1);
        TickType_t now = xTaskGetTickCount();

        if (reading_on != candidate_on) {
            // The reading moved: start timing it again
            candidate_on = reading_on;
            candidate_since = now;
        } else if (candidate_on != hook_on &&
                   pdTICKS_TO_MS(now - candidate_since) >= HOOK_DEBOUNCE_MS) {
            hook_on = candidate_on;
            report_hook(hook_on);
            // Measured 2026-11-10: about 2KB never used, answering a ringing call
            ESP_LOGD(TAG, "Hook task stack: %u bytes never used",
                     (unsigned)uxTaskGetStackHighWaterMark(NULL));
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
    hook_on = candidate_on = (gpio_get_level(PIN_OFF_HOOK_DETECT) == 1);
    candidate_since = xTaskGetTickCount();
    ESP_LOGI(TAG, "Initial hook state: %s", hook_on ? "on-hook" : "off-hook");
    if (!hook_on) {
        report_hook(hook_on);
    }

    // Create monitoring task
    BaseType_t task_ret = xTaskCreate(
        slic_monitor_task,
        "slic_monitor",
        SLIC_MONITOR_STACK_SIZE,
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
