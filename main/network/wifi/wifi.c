#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "app/events/event_system.h"
#include "app/state/ma_bell_state.h"

static const char *TAG = "wifi";

static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group = NULL;

static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // WiFi driver is ready, initiate connection
        // Coexistence is handled by initialization sequencing (BT starts after WiFi)
        ESP_LOGI(TAG, "WiFi started, initiating connection...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGW(TAG, "Disconnect from AP, reason: %d", disconnected->reason);

        // Clear WiFi connection state
        ma_bell_state_update_network_bits(0, NET_STATE_WIFI_CONNECTED);
        ma_bell_state_set_ip_address("0.0.0.0");
        ma_bell_state_set_wifi_info(0, 0);

        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP (attempt %d/%d)", s_retry_num, WIFI_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to AP after %d attempts", WIFI_MAXIMUM_RETRY);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "WiFi connected to AP, waiting for IP address...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        // Update state with IP address
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
        ma_bell_state_set_ip_address(ip_str);
        ma_bell_state_update_network_bits(NET_STATE_WIFI_CONNECTED, 0);

        // Get and update WiFi info (RSSI, channel)
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            ma_bell_state_set_wifi_info(ap_info.rssi, ap_info.primary);
        }
    }
}

esp_err_t wifi_init_sta(EventGroupHandle_t wifi_event_group)
{
    if (wifi_event_group == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    s_wifi_event_group = wifi_event_group;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &event_handler,
                                                      NULL,
                                                      &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &event_handler,
                                                      NULL,
                                                      &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Retrieve credentials from NVS and set config BEFORE starting WiFi
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASS_LEN];

    esp_err_t ret = wifi_get_credentials(ssid, sizeof(ssid), password, sizeof(password));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No WiFi credentials found in NVS storage!");
        ESP_LOGE(TAG, "Please provision WiFi credentials (see WIFI_SETUP.md)");
        return ESP_ERR_NVS_NOT_FOUND;
    }

    ESP_LOGI(TAG, "Found WiFi credentials in NVS for SSID: %s", ssid);

    // Set WiFi config
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_OPEN,  // Accept any security protocol (negotiated)
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    strlcpy((char *) wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char *) wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_LOGI(TAG, "WiFi config set for SSID: %s", ssid);

    // Start WiFi - this will trigger WIFI_EVENT_STA_START
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    return ESP_OK;
}

esp_err_t wifi_set_credentials(const char* ssid, const char* password)
{
    if (!ssid || !password) {
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(ssid) > MAX_SSID_LEN || strlen(password) > MAX_PASS_LEN) {
        return ESP_ERR_INVALID_SIZE;
    }

    esp_err_t ret = storage_set_str(STORAGE_NAMESPACE_WIFI, STORAGE_KEY_WIFI_SSID, ssid);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = storage_set_str(STORAGE_NAMESPACE_WIFI, STORAGE_KEY_WIFI_PASS, password);
    if (ret != ESP_OK) {
        return ret;
    }

    return storage_commit(STORAGE_NAMESPACE_WIFI);
}

esp_err_t wifi_get_credentials(char* ssid, size_t ssid_len, char* password, size_t pass_len)
{
    if (!ssid || !password || ssid_len < MAX_SSID_LEN || pass_len < MAX_PASS_LEN) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = storage_get_str(STORAGE_NAMESPACE_WIFI, STORAGE_KEY_WIFI_SSID, ssid, ssid_len);
    if (ret != ESP_OK) {
        return ret;
    }

    return storage_get_str(STORAGE_NAMESPACE_WIFI, STORAGE_KEY_WIFI_PASS, password, pass_len);
}

esp_err_t wifi_connect(void)
{
    // This function is now a no-op
    // WiFi config is set during wifi_init_sta() before esp_wifi_start()
    // Connection is initiated by the WIFI_EVENT_STA_START event handler
    ESP_LOGI(TAG, "wifi_connect() called - config already set during wifi_init_sta()");
    return ESP_OK;
}

esp_err_t wifi_disconnect(void)
{
    esp_err_t ret = esp_wifi_disconnect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disconnect WiFi");
        return ret;
    }

    return ESP_OK;
}