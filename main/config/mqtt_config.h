#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

// MQTT settings are stored in NVS only (namespace "mqtt"), like WiFi credentials.
// They are issued by the Deevnet tenant (infra/deevnet-tenant-mabell) and written
// by tools/provision_tenant.py. No hardcoded broker, account or topics.

// Buffer sizes (including terminator)
#define MQTT_MAX_HOST_LEN           64
#define MQTT_MAX_USER_LEN           64
#define MQTT_MAX_PASS_LEN           128
#define MQTT_MAX_TOPIC_LEN          96
#define MQTT_MAX_CA_LEN             4000  // NVS string limit; a PEM root is ~2KB
#define MQTT_MAX_URI_LEN            96

// Connection parameters
#define MQTT_DEFAULT_PORT           8883
#define MQTT_KEEPALIVE_SEC          60
#define MQTT_RECONNECT_TIMEOUT_MS   10000

// State publishing
#define MQTT_STATE_QOS              1
#define MQTT_STATE_POLL_MS          200   // How often the state is checked for changes
#define MQTT_LOG_QOS                1     // Events forwarded to the log topic
#define MQTT_STATE_TASK_STACK       4096
#define MQTT_STATE_TASK_PRIORITY    3
#define MQTT_STATE_PAYLOAD_LEN      384

#endif /* __MQTT_CONFIG_H__ */
