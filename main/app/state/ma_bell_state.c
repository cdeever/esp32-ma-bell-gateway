#include "ma_bell_state.h"
#include "app/events/event_log.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
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

// The state changes worth keeping in the tenant's log store: one event when a
// bit is set and one when it is cleared. Bits not listed here change silently.
typedef struct {
    uint8_t bit;
    const char *set_event;
    const char *set_msg;
    const char *clear_event;
    const char *clear_msg;
} state_event_t;

static const state_event_t phone_events[] = {
    { PHONE_STATE_OFF_HOOK, "phone.off_hook", "Handset lifted", "phone.on_hook", "Handset replaced" },
    { PHONE_STATE_DIALING, "phone.dialing_start", "Dialing started", "phone.dialing_stop", "Dialing finished" },
};

static const state_event_t bluetooth_events[] = {
    { BT_STATE_AUDIO_CONNECTED, "bt.audio_connected", "Call audio connected", "bt.audio_disconnected", "Call audio disconnected" },
};

static void log_state_events(const state_event_t *events, size_t count, uint8_t old_state, uint8_t new_state) {
    uint8_t changed = old_state ^ new_state;
    for (size_t i = 0; i < count; i++) {
        if (!(changed & events[i].bit)) {
            continue;
        }
        if (new_state & events[i].bit) {
            event_log(EVENT_LOG_INFO, events[i].set_event, "%s", events[i].set_msg);
        } else {
            event_log(EVENT_LOG_INFO, events[i].clear_event, "%s", events[i].clear_msg);
        }
    }
}

// The changes below say who or what, so each is written out rather than taken
// from a table. The details they report are set before the bit is updated.

static const char *bt_device_label(void) {
    if (g_state.bluetooth.device_name[0] != '\0') {
        return g_state.bluetooth.device_name;
    }
    return g_state.bluetooth.device_addr[0] != '\0' ? g_state.bluetooth.device_addr : "Mobile phone";
}

static void log_bt_connection(bool connected) {
    char name[2 * sizeof(g_state.bluetooth.device_name)];
    char extra[128];
    event_log_escape(g_state.bluetooth.device_name, name, sizeof(name));
    snprintf(extra, sizeof(extra), "\"phone\":\"%s\",\"phone_addr\":\"%s\"", name, g_state.bluetooth.device_addr);

    if (connected) {
        event_log_with(EVENT_LOG_INFO, "bt.connected", extra, "%s connected", bt_device_label());
    } else {
        event_log_with(EVENT_LOG_INFO, "bt.disconnected", extra, "%s disconnected", bt_device_label());
    }
}

// Ringing is the mobile phone's incoming call, passed on to the telephone. The
// caller's number arrives a moment after ringing starts, so it is known when
// ringing stops and usually not when it starts; call.incoming reports it then.
static void log_ringing(bool started) {
    const char *number = g_state.call.number;
    char name[2 * sizeof(g_state.bluetooth.device_name)];
    char escaped[2 * sizeof(g_state.call.number)];
    char extra[200];
    event_log_escape(g_state.bluetooth.device_name, name, sizeof(name));
    event_log_escape(number, escaped, sizeof(escaped));
    snprintf(extra, sizeof(extra), "\"phone\":\"%s\",\"phone_addr\":\"%s\",\"number\":\"%s\"",
             name, g_state.bluetooth.device_addr, escaped);

    const char *event = started ? "phone.ringing_start" : "phone.ringing_stop";
    const char *what = started ? "Ringing" : "Stopped ringing";
    if (number[0] != '\0') {
        event_log_with(EVENT_LOG_INFO, event, extra, "%s: call from %s on %s", what, number, bt_device_label());
    } else {
        event_log_with(EVENT_LOG_INFO, event, extra, "%s: incoming call on %s", what, bt_device_label());
    }
}

static void log_call(bool started) {
    const char *number = g_state.call.number;
    const char *direction = g_state.call.incoming ? "incoming" : "outgoing";
    char name[2 * sizeof(g_state.bluetooth.device_name)];
    char escaped[2 * sizeof(g_state.call.number)];
    char extra[200];
    event_log_escape(g_state.bluetooth.device_name, name, sizeof(name));
    event_log_escape(number, escaped, sizeof(escaped));

    if (started) {
        g_state.call.started_ms = esp_timer_get_time() / 1000;
        if (number[0] != '\0') {
            snprintf(extra, sizeof(extra), "\"number\":\"%s\",\"direction\":\"%s\",\"phone\":\"%s\"",
                     escaped, direction, name);
            event_log_with(EVENT_LOG_INFO, "call.started", extra, "Call started with %s (%s) on %s",
                           number, direction, bt_device_label());
        } else {
            snprintf(extra, sizeof(extra), "\"phone\":\"%s\"", name);
            event_log_with(EVENT_LOG_INFO, "call.started", extra, "Call started on %s", bt_device_label());
        }
        return;
    }

    long duration_s = 0;
    if (g_state.call.started_ms > 0) {
        duration_s = (long)((esp_timer_get_time() / 1000 - g_state.call.started_ms) / 1000);
    }
    if (number[0] != '\0') {
        snprintf(extra, sizeof(extra), "\"number\":\"%s\",\"direction\":\"%s\",\"duration_s\":%ld,\"phone\":\"%s\"",
                 escaped, direction, duration_s, name);
        event_log_with(EVENT_LOG_INFO, "call.ended", extra, "Call with %s on %s ended after %ld s",
                       number, bt_device_label(), duration_s);
    } else {
        snprintf(extra, sizeof(extra), "\"duration_s\":%ld,\"phone\":\"%s\"", duration_s, name);
        event_log_with(EVENT_LOG_INFO, "call.ended", extra, "Call on %s ended after %ld s", bt_device_label(), duration_s);
    }
    ma_bell_state_clear_call_party();
}

static void log_wifi(bool connected) {
    char ssid[2 * sizeof(g_state.network.ssid)];
    char extra[160];
    event_log_escape(g_state.network.ssid, ssid, sizeof(ssid));

    if (connected) {
        snprintf(extra, sizeof(extra), "\"ssid\":\"%s\",\"ip\":\"%s\",\"rssi\":%d,\"channel\":%d",
                 ssid, g_state.network.ip_address, -(int)g_state.network.rssi, (int)g_state.network.channel);
        event_log_with(EVENT_LOG_INFO, "wifi.connected", extra, "WiFi connected to %s as %s",
                       g_state.network.ssid, g_state.network.ip_address);
    } else {
        snprintf(extra, sizeof(extra), "\"ssid\":\"%s\",\"reason\":%d", ssid, (int)g_state.network.disconnect_reason);
        event_log_with(EVENT_LOG_WARN, "wifi.disconnected", extra, "WiFi disconnected from %s (reason %d)",
                       g_state.network.ssid, (int)g_state.network.disconnect_reason);
    }
}

#define LOG_STATE_EVENTS(events, old_state, new_state) \
    log_state_events((events), sizeof(events) / sizeof((events)[0]), (old_state), (new_state))

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
        if ((old_state ^ g_state.phone.state) & PHONE_STATE_RINGING) {
            if (g_state.phone.state & PHONE_STATE_RINGING) {
                // A new call: whoever the last one was with no longer applies
                ma_bell_state_clear_call_party();
            }
            log_ringing(g_state.phone.state & PHONE_STATE_RINGING);
        }
        LOG_STATE_EVENTS(phone_events, old_state, g_state.phone.state);
        notify_state_change(NOTIFY_PHONE_STATE_CHANGED);
    }
}

void ma_bell_state_update_bluetooth_bits(uint8_t set_bits, uint8_t clear_bits) {
    uint8_t old_state = g_state.bluetooth.state;
    g_state.bluetooth.state = (g_state.bluetooth.state | set_bits) & ~clear_bits;
    
    if (old_state != g_state.bluetooth.state) {
        ESP_LOGI(TAG, "Bluetooth state changed: 0x%02" PRIx8 " -> 0x%02" PRIx8, old_state, g_state.bluetooth.state);
        uint8_t changed = old_state ^ g_state.bluetooth.state;
        if (changed & BT_STATE_CONNECTED) {
            log_bt_connection(g_state.bluetooth.state & BT_STATE_CONNECTED);
            if (!(g_state.bluetooth.state & BT_STATE_IN_CALL)) {
                ma_bell_state_clear_call_party();
            }
        }
        if (changed & BT_STATE_IN_CALL) {
            log_call(g_state.bluetooth.state & BT_STATE_IN_CALL);
        }
        LOG_STATE_EVENTS(bluetooth_events, old_state, g_state.bluetooth.state);
        notify_state_change(NOTIFY_BT_STATE_CHANGED);
    }
}

void ma_bell_state_update_network_bits(uint8_t set_bits, uint8_t clear_bits) {
    uint8_t old_state = g_state.network.state;
    g_state.network.state = (g_state.network.state | set_bits) & ~clear_bits;
    
    if (old_state != g_state.network.state) {
        ESP_LOGI(TAG, "Network state changed: 0x%02" PRIx8 " -> 0x%02" PRIx8, old_state, g_state.network.state);
        if ((old_state ^ g_state.network.state) & NET_STATE_WIFI_CONNECTED) {
            log_wifi(g_state.network.state & NET_STATE_WIFI_CONNECTED);
        }
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

void ma_bell_state_set_ip_address(const char* ip) {
    if (ip) {
        strncpy(g_state.network.ip_address, ip, sizeof(g_state.network.ip_address) - 1);
        g_state.network.ip_address[sizeof(g_state.network.ip_address) - 1] = '\0';
        ESP_LOGI(TAG, "IP address set to: %s", g_state.network.ip_address);
    }
}

void ma_bell_state_set_wifi_info(int8_t rssi, uint8_t channel) {
    g_state.network.rssi = (uint8_t)(-rssi);  // Convert negative RSSI to positive for display
    g_state.network.channel = channel;
    ESP_LOGI(TAG, "WiFi info: RSSI=%d, Channel=%d", rssi, channel);
}

void ma_bell_state_set_wifi_ssid(const char* ssid) {
    if (ssid) {
        strlcpy(g_state.network.ssid, ssid, sizeof(g_state.network.ssid));
    }
}

void ma_bell_state_set_wifi_disconnect_reason(uint8_t reason) {
    g_state.network.disconnect_reason = reason;
}

void ma_bell_state_set_bt_device_name(const char* name) {
    if (name) {
        bool learned_late = g_state.bluetooth.device_name[0] == '\0' && name[0] != '\0' &&
                            (g_state.bluetooth.state & BT_STATE_CONNECTED);
        strncpy(g_state.bluetooth.device_name, name, sizeof(g_state.bluetooth.device_name) - 1);
        g_state.bluetooth.device_name[sizeof(g_state.bluetooth.device_name) - 1] = '\0';
        ESP_LOGI(TAG, "BT device name set to: %s", g_state.bluetooth.device_name);

        if (learned_late) {
            char escaped[2 * sizeof(g_state.bluetooth.device_name)];
            char extra[128];
            event_log_escape(g_state.bluetooth.device_name, escaped, sizeof(escaped));
            snprintf(extra, sizeof(extra), "\"phone\":\"%s\",\"phone_addr\":\"%s\"", escaped, g_state.bluetooth.device_addr);
            event_log_with(EVENT_LOG_INFO, "bt.identified", extra, "Connected phone is %s", g_state.bluetooth.device_name);
        }
    }
}

void ma_bell_state_set_bt_device_addr(const char* addr) {
    if (addr) {
        strlcpy(g_state.bluetooth.device_addr, addr, sizeof(g_state.bluetooth.device_addr));
    }
}

void ma_bell_state_set_call_party(const char* number, bool incoming) {
    if (number == NULL || number[0] == '\0') {
        return;
    }
    if (strncmp(g_state.call.number, number, sizeof(g_state.call.number) - 1) == 0 && g_state.call.incoming == incoming) {
        return;  // Repeated with every ring
    }

    strlcpy(g_state.call.number, number, sizeof(g_state.call.number));
    g_state.call.incoming = incoming;

    char escaped[2 * sizeof(g_state.call.number)];
    char extra[128];
    event_log_escape(g_state.call.number, escaped, sizeof(escaped));
    snprintf(extra, sizeof(extra), "\"number\":\"%s\",\"direction\":\"%s\"", escaped, incoming ? "incoming" : "outgoing");
    if (incoming) {
        event_log_with(EVENT_LOG_INFO, "call.incoming", extra, "Incoming call from %s on %s",
                       g_state.call.number, bt_device_label());
    } else {
        event_log_with(EVENT_LOG_INFO, "call.outgoing", extra, "Outgoing call to %s on %s",
                       g_state.call.number, bt_device_label());
    }
}

void ma_bell_state_clear_call_party(void) {
    g_state.call.number[0] = '\0';
    g_state.call.incoming = false;
    g_state.call.started_ms = 0;
}

void ma_bell_state_set_bt_metrics(uint8_t volume, uint8_t signal, uint8_t battery) {
    if (volume != 0xFF) g_state.bluetooth.volume = volume;
    if (signal != 0xFF) g_state.bluetooth.signal_strength = signal;
    if (battery != 0xFF) g_state.bluetooth.battery_level = battery;
    ESP_LOGI(TAG, "BT metrics: vol=%d, sig=%d, bat=%d",
             g_state.bluetooth.volume,
             g_state.bluetooth.signal_strength,
             g_state.bluetooth.battery_level);
} 