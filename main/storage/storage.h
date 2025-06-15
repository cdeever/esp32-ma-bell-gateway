#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>

// Namespace for different types of data
#define STORAGE_NAMESPACE_WIFI "wifi"
#define STORAGE_NAMESPACE_BT   "bt"
#define STORAGE_NAMESPACE_SYS  "sys"

// Keys for WiFi configuration
#define STORAGE_KEY_WIFI_SSID "ssid"
#define STORAGE_KEY_WIFI_PASS "pass"

// Keys for Bluetooth configuration
#define STORAGE_KEY_BT_DEVICE_NAME "dev_name"
#define STORAGE_KEY_BT_PAIRED_DEV  "paired_dev"

// Keys for System configuration
#define STORAGE_KEY_SYS_VOLUME     "volume"
#define STORAGE_KEY_SYS_RING_VOL   "ring_vol"

/**
 * @brief Initialize the storage system
 * 
 * This function initializes NVS and creates necessary namespaces
 * 
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_init(void);

/**
 * @brief Store a string value in NVS
 * 
 * @param namespace NVS namespace
 * @param key Key for the value
 * @param value String value to store
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_set_str(const char* namespace, const char* key, const char* value);

/**
 * @brief Get a string value from NVS
 * 
 * @param namespace NVS namespace
 * @param key Key for the value
 * @param value Buffer to store the value
 * @param max_len Maximum length of the buffer
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_get_str(const char* namespace, const char* key, char* value, size_t max_len);

/**
 * @brief Store a uint8_t value in NVS
 * 
 * @param namespace NVS namespace
 * @param key Key for the value
 * @param value Value to store
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_set_u8(const char* namespace, const char* key, uint8_t value);

/**
 * @brief Get a uint8_t value from NVS
 * 
 * @param namespace NVS namespace
 * @param key Key for the value
 * @param value Pointer to store the value
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_get_u8(const char* namespace, const char* key, uint8_t* value);

/**
 * @brief Store a uint32_t value in NVS
 * 
 * @param namespace NVS namespace
 * @param key Key for the value
 * @param value Value to store
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_set_u32(const char* namespace, const char* key, uint32_t value);

/**
 * @brief Get a uint32_t value from NVS
 * 
 * @param namespace NVS namespace
 * @param key Key for the value
 * @param value Pointer to store the value
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_get_u32(const char* namespace, const char* key, uint32_t* value);

/**
 * @brief Delete a value from NVS
 * 
 * @param namespace NVS namespace
 * @param key Key for the value
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_delete(const char* namespace, const char* key);

/**
 * @brief Commit changes to NVS
 * 
 * @param namespace NVS namespace
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t storage_commit(const char* namespace);

#endif /* __STORAGE_H__ */ 