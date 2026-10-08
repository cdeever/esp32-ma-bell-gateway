#include "mqtt.h"
#include "config/mqtt_config.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "mqtt";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static mqtt_message_callback_t message_callback = NULL;
static mqtt_connected_callback_t connected_callback = NULL;
static volatile bool mqtt_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected");
            mqtt_connected = true;
            if (connected_callback) {
                connected_callback();
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected");
            mqtt_connected = false;
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Publish acknowledged by broker, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            if (message_callback && event->data_len > 0) {
                char *topic = strndup(event->topic, event->topic_len);
                char *data = strndup(event->data, event->data_len);
                message_callback(topic, data, event->data_len);
                free(topic);
                free(data);
            }
            break;
        case MQTT_EVENT_ERROR:
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGE(TAG, "MQTT connection refused by broker, code %d", event->error_handle->connect_return_code);
            } else {
                ESP_LOGE(TAG, "MQTT transport error: esp_err=0x%x, tls=0x%x, errno=%d",
                         event->error_handle->esp_tls_last_esp_err,
                         event->error_handle->esp_tls_stack_err,
                         event->error_handle->esp_transport_sock_errno);
            }
            break;
        default:
            ESP_LOGD(TAG, "Other event id: %d", event->event_id);
            break;
    }
}

esp_err_t mqtt_init(const mqtt_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = config->broker_uri,
                // No .transport: the URI scheme selects it, and esp-mqtt rejects both
                .port = config->port,
            },
            .verification = {
                .certificate = config->ca_cert_pem,
            }
        },
        .credentials = {
            .username = config->username,
            .authentication = {
                .password = config->password,
            },
            .client_id = config->client_id,
        },
        .session = {
            .keepalive = MQTT_KEEPALIVE_SEC,
            .last_will = {
                .topic = config->lwt_topic,
                .msg = config->lwt_msg,
                .qos = 1,
                .retain = true,
            },
        },
        .network = {
            .reconnect_timeout_ms = MQTT_RECONNECT_TIMEOUT_MS,
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!mqtt_client) {
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    
    return ESP_OK;
}

esp_err_t mqtt_start(void)
{
    if (!mqtt_client) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_mqtt_client_start(mqtt_client);
}

esp_err_t mqtt_stop(void)
{
    if (!mqtt_client) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_mqtt_client_stop(mqtt_client);
}

esp_err_t mqtt_subscribe(const char *topic, int qos)
{
    if (!mqtt_client || !topic) {
        return ESP_ERR_INVALID_ARG;
    }
    return esp_mqtt_client_subscribe(mqtt_client, topic, qos);
}

esp_err_t mqtt_publish(const char *topic, const char *data, size_t len, int qos, bool retain)
{
    if (!mqtt_client || !topic || !data) {
        return ESP_ERR_INVALID_ARG;
    }
    // esp_mqtt_client_publish returns a message id (>= 0), or a negative value on failure
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, data, len, qos, retain);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t mqtt_register_message_callback(mqtt_message_callback_t callback)
{
    if (!callback) {
        return ESP_ERR_INVALID_ARG;
    }
    message_callback = callback;
    return ESP_OK;
}

esp_err_t mqtt_register_connected_callback(mqtt_connected_callback_t callback)
{
    if (!callback) {
        return ESP_ERR_INVALID_ARG;
    }
    connected_callback = callback;
    return ESP_OK;
}

bool mqtt_is_connected(void)
{
    return mqtt_connected;
}
