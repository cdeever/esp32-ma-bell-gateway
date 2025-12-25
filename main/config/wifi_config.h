#ifndef __WIFI_CONFIG_H__
#define __WIFI_CONFIG_H__

// Connection parameters
#define WIFI_CONNECT_TIMEOUT        30    // seconds
#define WIFI_MAXIMUM_RETRY          3

// WiFi credentials (TODO: move to secure storage in future)
#define DEFAULT_WIFI_SSID           "YourSSID"
#define DEFAULT_WIFI_PASS           "YourPassword"

// Buffer sizes
#define MAX_SSID_LEN                32
#define MAX_PASS_LEN                64

// Event group bits
#define WIFI_CONNECTED_BIT          BIT0
#define WIFI_FAIL_BIT               BIT1

#endif /* __WIFI_CONFIG_H__ */
