#ifndef __BLUETOOTH_CONFIG_H__
#define __BLUETOOTH_CONFIG_H__

#include "freertos/FreeRTOS.h"

// Device configuration
#define BT_DEVICE_NAME              "MA BELL"
#define BT_PIN_CODE                 {'0', '0', '0', '0'}
#define BT_PIN_CODE_LEN             4

// Connection management
#define BT_RECONNECT_FAST_INTERVAL_MS   10000  // Retry this often just after the phone is lost
#define BT_RECONNECT_FAST_PERIOD_MS     300000 // ...for this long
#define BT_RECONNECT_SLOW_INTERVAL_MS   60000  // Retry this often after that
#define BT_CONNECTED_POLL_MS            2000   // How soon a lost connection is noticed
#define BT_CONNECT_ATTEMPT_TIMEOUT_MS   30000  // Give up waiting for an attempt's result
#define BT_LINK_CLOSE_WAIT_MS            15000  // Wait this long for a closed session's link to drop
#define BT_HELD_PROBE_INTERVAL_MS       10000  // How often a held-off phone is checked for reach

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
