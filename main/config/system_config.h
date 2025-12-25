#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__

#include "esp_log.h"

// NVS Configuration
#define SYSTEM_NVS_PARTITION        "nvs"

// System initialization order (documented for reference)
// 1. NVS
// 2. Event system
// 3. State management
// 4. Storage
// 5. Hardware (GPIO/PCM)
// 6. Bluetooth
// 7. WiFi
// 8. Web server

// Global system parameters
#define SYSTEM_LOG_LEVEL            ESP_LOG_INFO

#endif /* __SYSTEM_CONFIG_H__ */
