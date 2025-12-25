#ifndef __WIFI_CONFIG_H__
#define __WIFI_CONFIG_H__

// Connection parameters
#define WIFI_CONNECT_TIMEOUT        30    // seconds
#define WIFI_MAXIMUM_RETRY          5     // Increased for BT/WiFi coexistence

// WiFi credentials are stored in NVS only
// Use provisioning tool to set credentials (see WIFI_SETUP.md)
// No hardcoded defaults - credentials must be provisioned to NVS partition
// #define DEFAULT_WIFI_SSID           "YourSSID"  // No longer used
// #define DEFAULT_WIFI_PASS           "YourPassword"  // No longer used

// Buffer sizes
#define MAX_SSID_LEN                32
#define MAX_PASS_LEN                64

// Event group bits
#define WIFI_CONNECTED_BIT          BIT0
#define WIFI_FAIL_BIT               BIT1

#endif /* __WIFI_CONFIG_H__ */
