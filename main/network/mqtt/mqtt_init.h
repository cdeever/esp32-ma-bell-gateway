#ifndef __MQTT_INIT_H__
#define __MQTT_INIT_H__

#include "esp_err.h"

/**
 * @brief Initialize the MQTT subsystem and connect to the tenant's broker
 *
 * This function:
 * - Reads the broker address, account, CA and topics from NVS (namespace "mqtt")
 * - Connects over TLS, verifying the broker against the stored CA
 * - Publishes the gateway's state (retained) on connect and whenever it changes
 * - Registers a last will that marks the gateway offline
 *
 * The connection is made in the background once WiFi is up, however long
 * after boot that is, and re-established automatically.
 *
 * @return ESP_OK on success, and also when MQTT is not provisioned (the
 *         gateway works without it); ESP_FAIL on an internal error
 */
esp_err_t mqtt_init_and_start(void);

#endif /* __MQTT_INIT_H__ */
