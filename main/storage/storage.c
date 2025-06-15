#include "storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "storage";

// NVS handles for different namespaces
static nvs_handle_t wifi_handle;
static nvs_handle_t bt_handle;
static nvs_handle_t sys_handle;

esp_err_t storage_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "NVS partition was truncated and needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized successfully");

    // Open handles for each namespace
    ret = nvs_open(STORAGE_NAMESPACE_WIFI, NVS_READWRITE, &wifi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_open(STORAGE_NAMESPACE_BT, NVS_READWRITE, &bt_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for BT: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_open(STORAGE_NAMESPACE_SYS, NVS_READWRITE, &sys_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for SYS: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t storage_set_str(const char* namespace, const char* key, const char* value)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    ret = nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for namespace '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_str(nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error setting NVS value for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    ESP_LOGI(TAG, "Successfully stored value for key '%s' in namespace '%s'", key, namespace);
    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t storage_get_str(const char* namespace, const char* key, char* value, size_t max_len)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    ret = nvs_open(namespace, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for namespace '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }

    size_t required_size = 0;
    ret = nvs_get_str(nvs_handle, key, NULL, &required_size);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "Key '%s' not found in namespace '%s': %s", key, namespace, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    if (required_size > max_len) {
        ESP_LOGE(TAG, "Value too long for buffer (required: %d, available: %d)", required_size, max_len);
        nvs_close(nvs_handle);
        return ESP_ERR_NVS_INVALID_LENGTH;
    }

    ret = nvs_get_str(nvs_handle, key, value, &max_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error getting NVS value for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    ESP_LOGI(TAG, "Successfully retrieved value for key '%s' from namespace '%s'", key, namespace);
    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t storage_set_u8(const char* namespace, const char* key, uint8_t value)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    ret = nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for namespace '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_u8(nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error setting uint8 value for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t storage_get_u8(const char* namespace, const char* key, uint8_t* value)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    ret = nvs_open(namespace, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for namespace '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_get_u8(nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error getting uint8 value for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t storage_set_u32(const char* namespace, const char* key, uint32_t value)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    ret = nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for namespace '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_u32(nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error setting uint32 value for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t storage_get_u32(const char* namespace, const char* key, uint32_t* value)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    ret = nvs_open(namespace, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for namespace '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_get_u32(nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error getting uint32 value for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t storage_delete(const char* namespace, const char* key)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    ret = nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for namespace '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_erase_key(nvs_handle, key);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error deleting key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS after delete for key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t storage_commit(const char* namespace)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    ret = nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for namespace '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS for namespace '%s': %s", namespace, esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
} 