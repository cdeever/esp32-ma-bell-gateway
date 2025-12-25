#ifndef __WEB_CONFIG_H__
#define __WEB_CONFIG_H__

// HTTP server configuration
#define WEB_SERVER_PORT             80
#define WEB_SERVER_CTRL_PORT        32768
#define WEB_SERVER_STACK_SIZE       8192
#define WEB_SERVER_CORE_ID          0
#define WEB_SERVER_MAX_URI_HANDLERS 8
#define WEB_SERVER_MAX_RESP_HEADERS 8
#define WEB_SERVER_BACKLOG_CONN     5

// Timeouts (in seconds)
#define WEB_SERVER_SEND_TIMEOUT     5
#define WEB_SERVER_RECV_TIMEOUT     5

// Status update interval (milliseconds)
#define WEB_STATUS_UPDATE_INTERVAL  2000

// Mutex timeout (milliseconds)
#define WEB_MUTEX_TIMEOUT_MS        1000

#endif /* __WEB_CONFIG_H__ */
