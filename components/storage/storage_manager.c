#include "storage_manager.h"
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "esp_timer.h"

#define TAG "storage"

#define KEY_REED_CLOSED_PIN "reed_closed"
#define KEY_REED_OPEN_PIN "reed_open"
#define KEY_RELAY_PIN "relay"
#define KEY_PULSE_DURATION "pulse_dur"
#define KEY_MAX_PULSE_DURATION "max_pulse"
#define KEY_MIN_INTERVAL "min_int"
#define KEY_DOOR_STATE "door_state"
#define KEY_EVENT_COUNT "evt_count"

#define MAX_EVENT_LOGS 100

static bool s_initialized = false;
static nvs_handle_t s_nvs_handle = 0;

esp_err_t storage_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &s_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    s_initialized = true;
    ESP_LOGI(TAG, "Initialized");
    return ESP_OK;
}

esp_err_t storage_save_gpio_config(const storage_gpio_config_t *config)
{
    if (!s_initialized || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nvs_set_u32(s_nvs_handle, KEY_REED_CLOSED_PIN, config->reed_closed_pin);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_set_u32(s_nvs_handle, KEY_REED_OPEN_PIN, config->reed_open_pin);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_set_u32(s_nvs_handle, KEY_RELAY_PIN, config->relay_pin);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_commit(s_nvs_handle);
    ESP_LOGI(TAG, "Saved GPIO config");
    return ret;
}

esp_err_t storage_load_gpio_config(storage_gpio_config_t *config)
{
    if (!s_initialized || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nvs_get_u32(s_nvs_handle, KEY_REED_CLOSED_PIN, &config->reed_closed_pin);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        return ret;
    }
    
    ret = nvs_get_u32(s_nvs_handle, KEY_REED_OPEN_PIN, &config->reed_open_pin);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        return ret;
    }
    
    ret = nvs_get_u32(s_nvs_handle, KEY_RELAY_PIN, &config->relay_pin);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        return ret;
    }
    
    ESP_LOGI(TAG, "Loaded GPIO config");
    return ESP_OK;
}

esp_err_t storage_save_relay_config(const storage_relay_config_t *config)
{
    if (!s_initialized || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nvs_set_u32(s_nvs_handle, KEY_PULSE_DURATION, config->pulse_duration_ms);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_set_u32(s_nvs_handle, KEY_MAX_PULSE_DURATION, config->max_pulse_duration_ms);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_set_u32(s_nvs_handle, KEY_MIN_INTERVAL, config->min_interval_ms);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_commit(s_nvs_handle);
    ESP_LOGI(TAG, "Saved relay config");
    return ret;
}

esp_err_t storage_load_relay_config(storage_relay_config_t *config)
{
    if (!s_initialized || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nvs_get_u32(s_nvs_handle, KEY_PULSE_DURATION, &config->pulse_duration_ms);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        return ret;
    }
    
    ret = nvs_get_u32(s_nvs_handle, KEY_MAX_PULSE_DURATION, &config->max_pulse_duration_ms);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        return ret;
    }
    
    ret = nvs_get_u32(s_nvs_handle, KEY_MIN_INTERVAL, &config->min_interval_ms);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        return ret;
    }
    
    ESP_LOGI(TAG, "Loaded relay config");
    return ESP_OK;
}

esp_err_t storage_save_door_state(uint32_t state)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nvs_set_u32(s_nvs_handle, KEY_DOOR_STATE, state);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_commit(s_nvs_handle);
    ESP_LOGI(TAG, "Saved door state: %" PRIu32, state);
    return ret;
}

esp_err_t storage_load_door_state(uint32_t *state)
{
    if (!s_initialized || !state) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nvs_get_u32(s_nvs_handle, KEY_DOOR_STATE, state);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        *state = DOOR_STATE_UNKNOWN;
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Loaded door state: %" PRIu32, *state);
    return ret;
}

esp_err_t storage_log_event(event_type_t type, int32_t value)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t count = 0;
    nvs_get_u32(s_nvs_handle, KEY_EVENT_COUNT, &count);
    
    char key[32];
    snprintf(key, sizeof(key), "evt_%" PRIu32, count);
    
    event_log_t log = {
        .type = type,
        .timestamp = esp_timer_get_time() / 1000,
        .value = value
    };
    
    esp_err_t ret = nvs_set_blob(s_nvs_handle, key, &log, sizeof(event_log_t));
    if (ret != ESP_OK) return ret;
    
    count = (count + 1) % MAX_EVENT_LOGS;
    ret = nvs_set_u32(s_nvs_handle, KEY_EVENT_COUNT, count);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_commit(s_nvs_handle);
    return ret;
}

esp_err_t storage_get_logs(event_log_t *logs, size_t max_count, size_t *actual_count)
{
    if (!s_initialized || !logs || !actual_count) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t count = 0;
    esp_err_t ret = nvs_get_u32(s_nvs_handle, KEY_EVENT_COUNT, &count);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        *actual_count = 0;
        return ESP_OK;
    }
    if (ret != ESP_OK) return ret;
    
    size_t to_read = count > max_count ? max_count : count;
    *actual_count = 0;
    
    for (size_t i = 0; i < to_read; i++) {
        char key[32];
        snprintf(key, sizeof(key), "evt_%" PRIu32, i);
        
        size_t size = sizeof(event_log_t);
        ret = nvs_get_blob(s_nvs_handle, key, &logs[*actual_count], &size);
        if (ret == ESP_OK) {
            (*actual_count)++;
        }
    }
    
    return ESP_OK;
}

esp_err_t storage_factory_reset(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_close(s_nvs_handle);
    esp_err_t ret = nvs_flash_erase();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Factory reset failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = nvs_flash_init();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &s_nvs_handle);
    
    ESP_LOGW(TAG, "Factory reset completed");
    return ret;
}