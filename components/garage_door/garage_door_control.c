#include "garage_door_control.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "reed_switch.h"
#include "relay_control.h"
#include "storage_manager.h"

#define TAG "garage_door"
#define DEFAULT_TIMEOUT_MS 30000
#define SAFETY_CHECK_INTERVAL_MS 100

static door_state_t s_current_state = DOOR_STATE_UNKNOWN;
static bool s_initialized = false;
static uint32_t s_timeout_ms = DEFAULT_TIMEOUT_MS;
static door_state_callback_t s_state_callback = NULL;
static SemaphoreHandle_t s_state_mutex = NULL;
static esp_timer_handle_t s_timeout_timer = NULL;
static esp_timer_handle_t s_safety_timer = NULL;
static TaskHandle_t s_safety_task = NULL;

static void update_state(door_state_t new_state)
{
    xSemaphoreTake(s_state_mutex, portMAX_DELAY);
    if (s_current_state != new_state) {
        ESP_LOGI(TAG, "State: %s -> %s", garage_door_state_to_string(s_current_state), garage_door_state_to_string(new_state));
        s_current_state = new_state;
        storage_save_door_state(new_state);
        
        if (s_state_callback) {
            s_state_callback(new_state);
        }
    }
    xSemaphoreGive(s_state_mutex);
}

static void timeout_timer_callback(void *arg)
{
    ESP_LOGW(TAG, "Operation timeout, stopping door");
    storage_log_event(EVENT_TYPE_TIMEOUT, s_current_state);
    update_state(DOOR_STATE_STOPPED);
}

static void safety_check_task(void *pvParameters)
{
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(SAFETY_CHECK_INTERVAL_MS));
        
        xSemaphoreTake(s_state_mutex, portMAX_DELAY);
        door_state_t state = s_current_state;
        xSemaphoreGive(s_state_mutex);
        
        if (state == DOOR_STATE_OPENING || state == DOOR_STATE_CLOSING) {
            door_position_t pos = reed_switch_get_position();
            
            if (state == DOOR_STATE_OPENING) {
                if (pos == DOOR_POSITION_CLOSED) {
                    ESP_LOGW(TAG, "Obstruction detected: door not opening");
                    storage_log_event(EVENT_TYPE_OBSTRUCTION, state);
                    update_state(DOOR_STATE_STOPPED);
                } else if (pos == DOOR_POSITION_OPEN) {
                    update_state(DOOR_STATE_OPEN);
                }
            } else if (state == DOOR_STATE_CLOSING) {
                if (pos == DOOR_POSITION_OPEN) {
                    ESP_LOGW(TAG, "Obstruction detected: door not closing");
                    storage_log_event(EVENT_TYPE_OBSTRUCTION, state);
                    update_state(DOOR_STATE_STOPPED);
                } else if (pos == DOOR_POSITION_CLOSED) {
                    update_state(DOOR_STATE_CLOSED);
                }
            }
        }
    }
}

static void reed_switch_callback(door_position_t position)
{
    xSemaphoreTake(s_state_mutex, portMAX_DELAY);
    door_state_t state = s_current_state;
    xSemaphoreGive(s_state_mutex);
    
    if (state == DOOR_STATE_OPENING || state == DOOR_STATE_CLOSING) {
        if (position == DOOR_POSITION_OPEN) {
            update_state(DOOR_STATE_OPEN);
        } else if (position == DOOR_POSITION_CLOSED) {
            update_state(DOOR_STATE_CLOSED);
        }
    }
}

esp_err_t garage_door_init(void)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    s_state_mutex = xSemaphoreCreateMutex();
    if (!s_state_mutex) {
        return ESP_ERR_NO_MEM;
    }
    
    uint32_t saved_state;
    if (storage_load_door_state(&saved_state) == ESP_OK) {
        s_current_state = (door_state_t)saved_state;
    } else {
        door_position_t pos = reed_switch_get_position();
        if (pos == DOOR_POSITION_CLOSED) {
            s_current_state = DOOR_STATE_CLOSED;
        } else if (pos == DOOR_POSITION_OPEN) {
            s_current_state = DOOR_STATE_OPEN;
        } else {
            s_current_state = DOOR_STATE_UNKNOWN;
        }
    }
    
    esp_timer_create_args_t timeout_args = {
        .callback = timeout_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "door_timeout"
    };
    
    esp_err_t ret = esp_timer_create(&timeout_args, &s_timeout_timer);
    if (ret != ESP_OK) {
        vSemaphoreDelete(s_state_mutex);
        return ret;
    }
    
    reed_switch_register_callback(reed_switch_callback);
    
    BaseType_t task_ret = xTaskCreate(safety_check_task, "safety", 2048, NULL, 7, &s_safety_task);
    if (task_ret != pdPASS) {
        esp_timer_delete(s_timeout_timer);
        vSemaphoreDelete(s_state_mutex);
        return ESP_ERR_NO_MEM;
    }
    
    s_initialized = true;
    ESP_LOGI(TAG, "Initialized, state: %s", garage_door_state_to_string(s_current_state));
    return ESP_OK;
}

esp_err_t garage_door_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (s_safety_task) {
        vTaskDelete(s_safety_task);
        s_safety_task = NULL;
    }
    
    if (s_timeout_timer) {
        esp_timer_stop(s_timeout_timer);
        esp_timer_delete(s_timeout_timer);
        s_timeout_timer = NULL;
    }
    
    vSemaphoreDelete(s_state_mutex);
    s_state_mutex = NULL;
    s_initialized = false;
    
    return ESP_OK;
}

esp_err_t garage_door_open(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(s_state_mutex, portMAX_DELAY);
    door_state_t state = s_current_state;
    xSemaphoreGive(s_state_mutex);
    
    if (state != DOOR_STATE_CLOSED && state != DOOR_STATE_STOPPED) {
        ESP_LOGW(TAG, "Cannot open from state %s", garage_door_state_to_string(state));
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = relay_activate();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to activate relay: %s", esp_err_to_name(ret));
        return ret;
    }
    
    update_state(DOOR_STATE_OPENING);
    esp_timer_start_once(s_timeout_timer, s_timeout_ms * 1000);
    storage_log_event(EVENT_TYPE_DOOR_OPEN, 0);
    
    return ESP_OK;
}

esp_err_t garage_door_close(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(s_state_mutex, portMAX_DELAY);
    door_state_t state = s_current_state;
    xSemaphoreGive(s_state_mutex);
    
    if (state != DOOR_STATE_OPEN && state != DOOR_STATE_STOPPED) {
        ESP_LOGW(TAG, "Cannot close from state %s", garage_door_state_to_string(state));
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = relay_activate();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to activate relay: %s", esp_err_to_name(ret));
        return ret;
    }
    
    update_state(DOOR_STATE_CLOSING);
    esp_timer_start_once(s_timeout_timer, s_timeout_ms * 1000);
    storage_log_event(EVENT_TYPE_DOOR_CLOSED, 0);
    
    return ESP_OK;
}

esp_err_t garage_door_stop(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(s_state_mutex, portMAX_DELAY);
    door_state_t state = s_current_state;
    xSemaphoreGive(s_state_mutex);
    
    if (state == DOOR_STATE_CLOSED || state == DOOR_STATE_OPEN) {
        return ESP_OK;
    }
    
    update_state(DOOR_STATE_STOPPED);
    
    if (s_timeout_timer) {
        esp_timer_stop(s_timeout_timer);
    }
    
    return ESP_OK;
}

door_state_t garage_door_get_state(void)
{
    door_state_t state;
    xSemaphoreTake(s_state_mutex, portMAX_DELAY);
    state = s_current_state;
    xSemaphoreGive(s_state_mutex);
    return state;
}

bool garage_door_is_moving(void)
{
    door_state_t state = garage_door_get_state();
    return state == DOOR_STATE_OPENING || state == DOOR_STATE_CLOSING;
}

esp_err_t garage_door_set_timeout(uint32_t timeout_ms)
{
    if (timeout_ms < 1000) {
        return ESP_ERR_INVALID_ARG;
    }
    s_timeout_ms = timeout_ms;
    return ESP_OK;
}

esp_err_t garage_door_register_state_callback(door_state_callback_t callback)
{
    if (!callback) {
        return ESP_ERR_INVALID_ARG;
    }
    s_state_callback = callback;
    return ESP_OK;
}

const char *garage_door_state_to_string(door_state_t state)
{
    switch (state) {
        case DOOR_STATE_CLOSED: return "CLOSED";
        case DOOR_STATE_OPENING: return "OPENING";
        case DOOR_STATE_OPEN: return "OPEN";
        case DOOR_STATE_CLOSING: return "CLOSING";
        case DOOR_STATE_STOPPED: return "STOPPED";
        case DOOR_STATE_UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}