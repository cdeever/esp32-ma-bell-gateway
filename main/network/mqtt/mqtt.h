#ifndef MQTT_H
#define MQTT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "mqtt_client.h"

// MQTT Configuration structure
typedef struct {
    const char *broker_uri;      // MQTT broker URI; the scheme (mqtt://, mqtts://) selects the transport
    const char *client_id;       // Client ID for this device
    const char *username;        // Optional username
    const char *password;        // Optional password
    uint32_t port;              // Broker port
    const char *ca_cert_pem;     // CA the broker is verified against (PEM); must outlive the client
    const char *lwt_topic;       // Optional last-will topic
    const char *lwt_msg;         // Last-will payload (retained, QoS 1)
} mqtt_config_t;

// MQTT message callback type
typedef void (*mqtt_message_callback_t)(const char *topic, const char *data, size_t len);

// Called from the MQTT task each time the broker connection is established
typedef void (*mqtt_connected_callback_t)(void);

/**
 * @brief Initialize MQTT client with given configuration
 * 
 * @param config MQTT configuration
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_init(const mqtt_config_t *config);

/**
 * @brief Start MQTT client and connect to broker
 * 
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_start(void);

/**
 * @brief Stop MQTT client and disconnect from broker
 * 
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_stop(void);

/**
 * @brief Subscribe to a topic
 * 
 * @param topic Topic to subscribe to
 * @param qos Quality of service (0, 1, or 2)
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_subscribe(const char *topic, int qos);

/**
 * @brief Publish message to a topic
 * 
 * @param topic Topic to publish to
 * @param data Message data
 * @param len Length of message data
 * @param qos Quality of service (0, 1, or 2)
 * @param retain Whether to retain the message
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_publish(const char *topic, const char *data, size_t len, int qos, bool retain);

/**
 * @brief Register callback for received messages
 * 
 * @param callback Function to call when message is received
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_register_message_callback(mqtt_message_callback_t callback);

/**
 * @brief Register callback for broker connection (including reconnections)
 *
 * @param callback Function to call when the client connects
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_register_connected_callback(mqtt_connected_callback_t callback);

/**
 * @brief Whether the client is currently connected to the broker
 */
bool mqtt_is_connected(void);

#endif // MQTT_H 