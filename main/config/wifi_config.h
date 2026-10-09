#ifndef __WIFI_CONFIG_H__
#define __WIFI_CONFIG_H__

// Connection parameters
#define WIFI_CONNECT_TIMEOUT        60    // Increased to allow all retries to complete
#define WIFI_MAXIMUM_RETRY          5     // Immediate retries at boot before startup continues without WiFi

// After the immediate retries, reconnection continues in the background with
// a delay that doubles from the minimum to the maximum
#define WIFI_RECONNECT_MIN_DELAY_MS 5000
#define WIFI_RECONNECT_MAX_DELAY_MS 60000

// WiFi credentials are stored in NVS only
// Use provisioning tool to set credentials (see WIFI_SETUP.md)
// No hardcoded defaults - credentials must be provisioned to NVS partition
// #define DEFAULT_WIFI_SSID           "YourSSID"  // No longer used
// #define DEFAULT_WIFI_PASS           "YourPassword"  // No longer used

// Hostname announced over DHCP: the gateway's name in the Deevnet device registry
#define WIFI_HOSTNAME               "ma-bell-gw-01"

// Buffer sizes
#define MAX_SSID_LEN                32
#define MAX_PASS_LEN                64

// Event group bits
#define WIFI_CONNECTED_BIT          BIT0
#define WIFI_FAIL_BIT               BIT1

#endif /* __WIFI_CONFIG_H__ */
