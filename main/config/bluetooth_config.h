#ifndef __BLUETOOTH_CONFIG_H__
#define __BLUETOOTH_CONFIG_H__

#include "freertos/FreeRTOS.h"

// Device configuration
#define BT_DEVICE_NAME              "MA BELL"
#define BT_PIN_CODE                 {'0', '0', '0', '0'}
#define BT_PIN_CODE_LEN             4

// Connection management
#define BT_RECONNECT_INTERVAL_MS    10000  // 10 seconds
#define BT_DISCOVERY_DURATION       10     // 10 * 1.28s = 12.8s
#define BT_DISCOVERY_MAX_RESPONSES  0      // Unlimited

// Task configuration
#define BT_APP_TASK_STACK_SIZE      2048
#define BT_APP_TASK_PRIORITY        (configMAX_PRIORITIES - 3)
#define BT_APP_TASK_QUEUE_SIZE      10
#define BT_APP_TASK_NAME            "BtAppT"

// Reconnection task configuration
#define BT_RECONNECT_TASK_STACK_SIZE 4096
#define BT_RECONNECT_TASK_PRIORITY   5
#define BT_RECONNECT_TASK_NAME       "bt_reconnect"

#endif /* __BLUETOOTH_CONFIG_H__ */
