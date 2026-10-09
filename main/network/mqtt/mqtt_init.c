/*
 * MQTT Subsystem Initialization
 *
 * Connects the gateway to its Deevnet tenant's broker and reports its state.
 */

#include "mqtt_init.h"
#include "mqtt.h"
#include "config/mqtt_config.h"
#include "config/wifi_config.h"
#include "storage/storage.h"
#include "app/state/ma_bell_state.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "MQTT_INIT";

// The client keeps pointers into these, so they live for the life of the program
static char s_host[MQTT_MAX_HOST_LEN];
static char s_user[MQTT_MAX_USER_LEN];
static char s_pass[MQTT_MAX_PASS_LEN];
static char s_state_topic[MQTT_MAX_TOPIC_LEN];
static char s_uri[MQTT_MAX_URI_LEN];
static char *s_ca = NULL;

static TaskHandle_t s_state_task = NULL;
static bool s_client_started = false;

static const char *OFFLINE_PAYLOAD = "{\"device\":\"" WIFI_HOSTNAME "\",\"online\":false}";

static esp_err_t mqtt_load_config(uint32_t *port)
{
    esp_err_t ret = storage_get_str(STORAGE_NAMESPACE_MQTT, STORAGE_KEY_MQTT_HOST, s_host, sizeof(s_host));
    if (ret != ESP_OK) {
        return ret;
    }
    ret = storage_get_str(STORAGE_NAMESPACE_MQTT, STORAGE_KEY_MQTT_USER, s_user, sizeof(s_user));
    if (ret != ESP_OK) {
        return ret;
    }
    ret = storage_get_str(STORAGE_NAMESPACE_MQTT, STORAGE_KEY_MQTT_PASS, s_pass, sizeof(s_pass));
    if (ret != ESP_OK) {
        return ret;
    }
    ret = storage_get_str(STORAGE_NAMESPACE_MQTT, STORAGE_KEY_MQTT_STATE_TOPIC, s_state_topic, sizeof(s_state_topic));
    if (ret != ESP_OK) {
        return ret;
    }

    s_ca = malloc(MQTT_MAX_CA_LEN);
    if (s_ca == NULL) {
        return ESP_ERR_NO_MEM;
    }
    ret = storage_get_str(STORAGE_NAMESPACE_MQTT, STORAGE_KEY_MQTT_CA, s_ca, MQTT_MAX_CA_LEN);
    if (ret != ESP_OK) {
        free(s_ca);
        s_ca = NULL;
        return ret;
    }

    if (storage_get_u32(STORAGE_NAMESPACE_MQTT, STORAGE_KEY_MQTT_PORT, port) != ESP_OK) {
        *port = MQTT_DEFAULT_PORT;
    }

    return ESP_OK;
}

static void mqtt_publish_state(void)
{
    const ma_bell_state_t *state = ma_bell_state_get();
    char payload[MQTT_STATE_PAYLOAD_LEN];

    int len = snprintf(payload, sizeof(payload),
        "{\"device\":\"" WIFI_HOSTNAME "\",\"online\":true,"
        "\"phone\":{\"off_hook\":%s,\"ringing\":%s,\"dialing\":%s},"
        "\"bluetooth\":{\"connected\":%s,\"in_call\":%s,\"audio\":%s},"
        "\"wifi\":{\"ip\":\"%s\",\"rssi\":%d},"
        "\"uptime_s\":%lu}",
        (state->phone.state & PHONE_STATE_OFF_HOOK) ? "true" : "false",
        (state->phone.state & PHONE_STATE_RINGING) ? "true" : "false",
        (state->phone.state & PHONE_STATE_DIALING) ? "true" : "false",
        (state->bluetooth.state & BT_STATE_CONNECTED) ? "true" : "false",
        (state->bluetooth.state & BT_STATE_IN_CALL) ? "true" : "false",
        (state->bluetooth.state & BT_STATE_AUDIO_CONNECTED) ? "true" : "false",
        state->network.ip_address,
        -(int)state->network.rssi,
        (unsigned long)(esp_timer_get_time() / 1000000));

    if (len < 0 || len >= (int)sizeof(payload)) {
        ESP_LOGE(TAG, "State payload does not fit in %d bytes", (int)sizeof(payload));
        return;
    }

    if (mqtt_publish(s_state_topic, payload, len, MQTT_STATE_QOS, true) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to publish state");
    }
}

// Runs in the MQTT task: wake the state task rather than publish from here
static void mqtt_on_connected(void)
{
    if (s_state_task != NULL) {
        xTaskNotifyGive(s_state_task);
    }
}

// Publishes the state on connect and whenever the reported bits change.
// Polls the state rather than subscribing to events: phone state changes are
// not published as events, and event callbacks run in the publisher's context.
static void mqtt_state_task(void *arg)
{
    const uint8_t phone_mask = PHONE_STATE_OFF_HOOK | PHONE_STATE_RINGING | PHONE_STATE_DIALING;
    const uint8_t bt_mask = BT_STATE_CONNECTED | BT_STATE_IN_CALL | BT_STATE_AUDIO_CONNECTED;
    uint8_t last_phone = 0;
    uint8_t last_bt = 0;

    while (1) {
        bool connected_now = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(MQTT_STATE_POLL_MS)) > 0;

        // The client is started the first time WiFi is up, which may be long
        // after boot; from then on it reconnects by itself
        if (!s_client_started && ma_bell_state_network_bits_set(NET_STATE_WIFI_CONNECTED)) {
            esp_err_t ret = mqtt_start();
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(ret));
                continue;
            }
            s_client_started = true;
            ESP_LOGI(TAG, "MQTT client started");
        }

        if (!mqtt_is_connected()) {
            continue;
        }

        const ma_bell_state_t *state = ma_bell_state_get();
        uint8_t phone = state->phone.state & phone_mask;
        uint8_t bt = state->bluetooth.state & bt_mask;

        if (connected_now || phone != last_phone || bt != last_bt) {
            last_phone = phone;
            last_bt = bt;
            mqtt_publish_state();
        }
    }
}

esp_err_t mqtt_init_and_start(void)
{
    ESP_LOGI(TAG, "Initializing MQTT subsystem");

    uint32_t port = MQTT_DEFAULT_PORT;
    esp_err_t ret = mqtt_load_config(&port);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "No MQTT settings found in NVS storage (%s), continuing without MQTT", esp_err_to_name(ret));
        ESP_LOGW(TAG, "Provision the tenant's broker account with tools/provision_tenant.py");
        return ESP_OK;
    }

    snprintf(s_uri, sizeof(s_uri), "mqtts://%s", s_host);

    mqtt_config_t config = {
        .broker_uri = s_uri,
        .client_id = s_user,
        .username = s_user,
        .password = s_pass,
        .port = port,
        .ca_cert_pem = s_ca,
        .lwt_topic = s_state_topic,
        .lwt_msg = OFFLINE_PAYLOAD,
    };

    ret = mqtt_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client: %s", esp_err_to_name(ret));
        return ret;
    }

    mqtt_register_connected_callback(mqtt_on_connected);

    if (xTaskCreate(mqtt_state_task, "mqtt_state", MQTT_STATE_TASK_STACK, NULL,
                    MQTT_STATE_TASK_PRIORITY, &s_state_task) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create MQTT state task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "MQTT client ready: %s:%lu as %s, state on %s; it connects once WiFi is up",
             s_host, (unsigned long)port, s_user, s_state_topic);
    return ESP_OK;
}
