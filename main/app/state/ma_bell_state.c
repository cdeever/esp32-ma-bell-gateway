#include "ma_bell_state.h"
#include "esp_log.h"
#include <string.h>
#include <inttypes.h>

static const char *TAG = "ma_bell_state";

// Global state instance
static ma_bell_state_t g_state = {0};

// List of tasks registered for notifications
#define MAX_NOTIFIED_TASKS 8
typedef struct {
    TaskHandle_t task;
    uint32_t notification_bits;
} notified_task_t;

static notified_task_t g_notified_tasks[MAX_NOTIFIED_TASKS] = {0};
static int g_notified_task_count = 0;

esp_err_t ma_bell_state_init(void) {
    memset(&g_state, 0, sizeof(g_state));
    
    // Initialize system state
    g_state.system.state = SYS_STATE_INITIALIZED;
    
    // Initialize other fields
    g_state.phone.last_digit = INVALID_DIGIT;
    g_state.bluetooth.volume = 8;  // Default to middle volume
    g_state.network.rssi = 0;
    g_state.system.battery_level = 100;
    
    ESP_LOGI(TAG, "State management system initialized");
    return ESP_OK;
}

const ma_bell_state_t* ma_bell_state_get(void) {
    return &g_state;
}

// Helper function to notify tasks of state changes
static void notify_state_change(uint32_t notification_bit) {
    for (int i = 0; i < g_notified_task_count; i++) {
        if (g_notified_tasks[i].notification_bits & notification_bit) {
            xTaskNotify(g_notified_tasks[i].task, notification_bit, eSetBits);
        }
    }
}

void ma_bell_state_update_phone_bits(uint8_t set_bits, uint8_t clear_bits) {
    uint8_t old_state = g_state.phone.state;
    g_state.phone.state = (g_state.phone.state | set_bits) & ~clear_bits;
    
    if (old_state != g_state.phone.state) {
        ESP_LOGI(TAG, "Phone state changed: 0x%02" PRIx8 " -> 0x%02" PRIx8, old_state, g_state.phone.state);
        notify_state_change(NOTIFY_PHONE_STATE_CHANGED);
    }
}

void ma_bell_state_update_bluetooth_bits(uint8_t set_bits, uint8_t clear_bits) {
    uint8_t old_state = g_state.bluetooth.state;
    g_state.bluetooth.state = (g_state.bluetooth.state | set_bits) & ~clear_bits;
    
    if (old_state != g_state.bluetooth.state) {
        ESP_LOGI(TAG, "Bluetooth state changed: 0x%02" PRIx8 " -> 0x%02" PRIx8, old_state, g_state.bluetooth.state);
        notify_state_change(NOTIFY_BT_STATE_CHANGED);
    }
}

void ma_bell_state_update_network_bits(uint8_t set_bits, uint8_t clear_bits) {
    uint8_t old_state = g_state.network.state;
    g_state.network.state = (g_state.network.state | set_bits) & ~clear_bits;
    
    if (old_state != g_state.network.state) {
        ESP_LOGI(TAG, "Network state changed: 0x%02" PRIx8 " -> 0x%02" PRIx8, old_state, g_state.network.state);
        notify_state_change(NOTIFY_NETWORK_STATE_CHANGED);
    }
}

void ma_bell_state_update_system_bits(uint8_t set_bits, uint8_t clear_bits) {
    uint8_t old_state = g_state.system.state;
    g_state.system.state = (g_state.system.state | set_bits) & ~clear_bits;
    
    if (old_state != g_state.system.state) {
        ESP_LOGI(TAG, "System state changed: 0x%02" PRIx8 " -> 0x%02" PRIx8, old_state, g_state.system.state);
        notify_state_change(NOTIFY_SYSTEM_STATE_CHANGED);
    }
}

int ma_bell_state_phone_bits_set(uint8_t bits) {
    return (g_state.phone.state & bits) == bits;
}

int ma_bell_state_bluetooth_bits_set(uint8_t bits) {
    return (g_state.bluetooth.state & bits) == bits;
}

int ma_bell_state_network_bits_set(uint8_t bits) {
    return (g_state.network.state & bits) == bits;
}

int ma_bell_state_system_bits_set(uint8_t bits) {
    return (g_state.system.state & bits) == bits;
}

esp_err_t ma_bell_state_register_for_notifications(uint32_t notification_bits) {
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    
    // Check if task is already registered
    for (int i = 0; i < g_notified_task_count; i++) {
        if (g_notified_tasks[i].task == current_task) {
            g_notified_tasks[i].notification_bits |= notification_bits;
            return ESP_OK;
        }
    }
    
    // Add new task registration
    if (g_notified_task_count >= MAX_NOTIFIED_TASKS) {
        ESP_LOGE(TAG, "Too many tasks registered for notifications");
        return ESP_ERR_NO_MEM;
    }
    
    g_notified_tasks[g_notified_task_count].task = current_task;
    g_notified_tasks[g_notified_task_count].notification_bits = notification_bits;
    g_notified_task_count++;
    
    ESP_LOGI(TAG, "Task registered for notifications: 0x%08" PRIx32, notification_bits);
    return ESP_OK;
}

uint32_t ma_bell_state_wait_for_notification(uint32_t notification_bits, uint32_t timeout_ms) {
    uint32_t notification_value = 0;
    BaseType_t result;
    
    if (timeout_ms == 0) {
        result = xTaskNotifyWait(0, notification_bits, &notification_value, portMAX_DELAY);
    } else {
        result = xTaskNotifyWait(0, notification_bits, &notification_value, pdMS_TO_TICKS(timeout_ms));
    }
    
    if (result != pdTRUE) {
        return 0;  // Timeout
    }
    
    return notification_value & notification_bits;
} 