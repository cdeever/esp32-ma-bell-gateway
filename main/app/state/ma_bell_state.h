#ifndef __MA_BELL_STATE_H__
#define __MA_BELL_STATE_H__

/**
 * @file ma_bell_state.h
 * @brief Centralized state management system for the Ma Bell Gateway
 * 
 * This module provides a centralized way to track and manage the system's state
 * using bitmasks for efficient state representation and FreeRTOS task notifications
 * for state change awareness.
 * 
 * For detailed implementation information and thread safety considerations,
 * see the documentation in docs/source/implementation/state_management.rst
 */

#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Phone state bitmasks
#define PHONE_STATE_OFF_HOOK     (1 << 0)
#define PHONE_STATE_RINGING      (1 << 1)
#define PHONE_STATE_DIALING      (1 << 2)
#define PHONE_STATE_DIAL_TONE    (1 << 3)
#define PHONE_STATE_BUSY_TONE    (1 << 4)
#define PHONE_STATE_REORDER_TONE (1 << 5)
#define PHONE_STATE_RINGBACK     (1 << 6)
#define PHONE_STATE_CALL_WAITING (1 << 7)

// Bluetooth state bitmasks
#define BT_STATE_CONNECTED       (1 << 0)
#define BT_STATE_IN_CALL         (1 << 1)
#define BT_STATE_AUDIO_CONNECTED (1 << 2)
#define BT_STATE_MIC_MUTED       (1 << 3)
#define BT_STATE_SPEAKER_MUTED   (1 << 4)
#define BT_STATE_VOLUME_SYNC     (1 << 5)
#define BT_STATE_AG_FEATURES     (1 << 6)
#define BT_STATE_HF_FEATURES     (1 << 7)

// Network state bitmasks
#define NET_STATE_WIFI_CONNECTED (1 << 0)
#define NET_STATE_IP_ACQUIRED    (1 << 1)
#define NET_STATE_DNS_READY      (1 << 2)
#define NET_STATE_WEBSERVER_UP   (1 << 3)
#define NET_STATE_MDNS_READY     (1 << 4)

// System state bitmasks
#define SYS_STATE_INITIALIZED    (1 << 0)
#define SYS_STATE_ERROR          (1 << 1)
#define SYS_STATE_LOW_BATTERY    (1 << 2)
#define SYS_STATE_CHARGING       (1 << 3)
#define SYS_STATE_FACTORY_RESET  (1 << 4)
#define SYS_STATE_OTA_AVAILABLE  (1 << 5)

// Invalid digit marker
#define INVALID_DIGIT 0xFF

// Notification bits for state changes
#define NOTIFY_PHONE_STATE_CHANGED    (1 << 0)
#define NOTIFY_BT_STATE_CHANGED       (1 << 1)
#define NOTIFY_NETWORK_STATE_CHANGED  (1 << 2)
#define NOTIFY_SYSTEM_STATE_CHANGED   (1 << 3)

/**
 * @brief Structure containing the current state of the Ma Bell system
 */
typedef struct {
    struct {
        uint8_t state;           // Phone state bitmask
        uint8_t last_digit;      // Last digit dialed (0-9, INVALID_DIGIT if none)
        uint8_t ring_count;      // Number of rings in current cycle
        uint8_t dial_timeout;    // Timeout counter for dialing
    } phone;

    struct {
        uint8_t state;           // Bluetooth state bitmask
        uint8_t volume;          // Current volume level (0-15)
        uint8_t signal_strength; // Signal strength (0-5)
        uint8_t battery_level;   // Battery level (0-5)
        char device_name[32];    // Name of connected device
    } bluetooth;

    struct {
        uint8_t state;           // Network state bitmask
        uint8_t rssi;            // WiFi signal strength
        uint8_t channel;         // WiFi channel
        char ip_address[16];     // IP address if connected
        char hostname[32];       // mDNS hostname
    } network;

    struct {
        uint8_t state;           // System state bitmask
        uint8_t error_code;      // Last error code
        uint8_t battery_level;   // System battery level (0-100)
        uint8_t temperature;     // System temperature
    } system;
} ma_bell_state_t;

/**
 * @brief Initialize the state management system
 * 
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t ma_bell_state_init(void);

/**
 * @brief Get the current state of the system
 * 
 * @return Pointer to the current state structure
 */
const ma_bell_state_t* ma_bell_state_get(void);

/**
 * @brief Update phone state bits and notify waiting tasks
 * 
 * @param set_bits Bits to set
 * @param clear_bits Bits to clear
 */
void ma_bell_state_update_phone_bits(uint8_t set_bits, uint8_t clear_bits);

/**
 * @brief Update bluetooth state bits and notify waiting tasks
 * 
 * @param set_bits Bits to set
 * @param clear_bits Bits to clear
 */
void ma_bell_state_update_bluetooth_bits(uint8_t set_bits, uint8_t clear_bits);

/**
 * @brief Update network state bits and notify waiting tasks
 * 
 * @param set_bits Bits to set
 * @param clear_bits Bits to clear
 */
void ma_bell_state_update_network_bits(uint8_t set_bits, uint8_t clear_bits);

/**
 * @brief Update system state bits and notify waiting tasks
 * 
 * @param set_bits Bits to set
 * @param clear_bits Bits to clear
 */
void ma_bell_state_update_system_bits(uint8_t set_bits, uint8_t clear_bits);

/**
 * @brief Check if specific phone state bits are set
 * 
 * @param bits Bits to check
 * @return 1 if all specified bits are set, 0 otherwise
 */
int ma_bell_state_phone_bits_set(uint8_t bits);

/**
 * @brief Check if specific bluetooth state bits are set
 * 
 * @param bits Bits to check
 * @return 1 if all specified bits are set, 0 otherwise
 */
int ma_bell_state_bluetooth_bits_set(uint8_t bits);

/**
 * @brief Check if specific network state bits are set
 * 
 * @param bits Bits to check
 * @return 1 if all specified bits are set, 0 otherwise
 */
int ma_bell_state_network_bits_set(uint8_t bits);

/**
 * @brief Check if specific system state bits are set
 * 
 * @param bits Bits to check
 * @return 1 if all specified bits are set, 0 otherwise
 */
int ma_bell_state_system_bits_set(uint8_t bits);

/**
 * @brief Register current task to receive state change notifications
 * 
 * @param notification_bits Bitmask of notifications to receive
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t ma_bell_state_register_for_notifications(uint32_t notification_bits);

/**
 * @brief Wait for state change notification
 * 
 * @param notification_bits Bitmask of notifications to wait for
 * @param timeout_ms Timeout in milliseconds (0 for no timeout)
 * @return Bitmask of received notifications, 0 on timeout
 */
uint32_t ma_bell_state_wait_for_notification(uint32_t notification_bits, uint32_t timeout_ms);

#endif /* __MA_BELL_STATE_H__ */ 