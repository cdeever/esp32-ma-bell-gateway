#ifndef MQTT_H
#define MQTT_H

#include "esp_err.h"
#include "mqtt_client.h"

// MQTT Configuration structure
typedef struct {
    const char *broker_uri;      // MQTT broker URI
    const char *client_id;       // Client ID for this device
    const char *username;        // Optional username
    const char *password;        // Optional password
    uint32_t port;              // Broker port
    bool use_ssl;               // Whether to use SSL/TLS
} mqtt_config_t;

// MQTT message callback type
typedef void (*mqtt_message_callback_t)(const char *topic, const char *data, size_t len);

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

#endif // MQTT_H 