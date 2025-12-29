#include "relay_control.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define DEFAULT_PULSE_DURATION_MS 500
#define DEFAULT_MAX_PULSE_DURATION_MS 600
#define DEFAULT_MIN_INTERVAL_MS 1000
#define TAG "relay"

static gpio_num_t s_gpio_num = GPIO_NUM_NC;
static bool s_initialized = false;
static bool s_active = false;
static esp_timer_handle_t s_pulse_timer = NULL;
static relay_config_t s_config = {
    .pulse_duration_ms = DEFAULT_PULSE_DURATION_MS,
    .max_pulse_duration_ms = DEFAULT_MAX_PULSE_DURATION_MS,
    .min_interval_ms = DEFAULT_MIN_INTERVAL_MS
};
static int64_t s_last_activation_time = 0;
static relay_callback_t s_callback = NULL;
static SemaphoreHandle_t s_mutex = NULL;

static void pulse_timer_callback(void *arg)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    gpio_set_level(s_gpio_num, 0);
    s_active = false;
    xSemaphoreGive(s_mutex);
    
    ESP_LOGI(TAG, "Pulse completed, relay deactivated");
    
    if (s_callback) {
        s_callback();
    }
}

esp_err_t relay_init(gpio_num_t gpio_num)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) {
        return ESP_ERR_NO_MEM;
    }
    
    s_gpio_num = gpio_num;
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        vSemaphoreDelete(s_mutex);
        return ret;
    }
    
    gpio_set_level(gpio_num, 0);
    
    esp_timer_create_args_t timer_args = {
        .callback = pulse_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "relay_pulse"
    };
    
    ret = esp_timer_create(&timer_args, &s_pulse_timer);
    if (ret != ESP_OK) {
        vSemaphoreDelete(s_mutex);
        return ret;
    }
    
    s_initialized = true;
    ESP_LOGI(TAG, "Initialized on GPIO %d", gpio_num);
    return ESP_OK;
}

esp_err_t relay_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    
    if (s_pulse_timer) {
        esp_timer_stop(s_pulse_timer);
        esp_timer_delete(s_pulse_timer);
        s_pulse_timer = NULL;
    }
    
    gpio_set_level(s_gpio_num, 0);
    s_active = false;
    s_initialized = false;
    
    xSemaphoreGive(s_mutex);
    vSemaphoreDelete(s_mutex);
    s_mutex = NULL;
    
    return ESP_OK;
}

esp_err_t relay_activate(void)
{
    return relay_activate_pulse(s_config.pulse_duration_ms);
}

esp_err_t relay_activate_pulse(uint32_t duration_ms)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (duration_ms == 0 || duration_ms > s_config.max_pulse_duration_ms) {
        return ESP_ERR_INVALID_ARG;
    }
    
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    
    if (s_active) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_STATE;
    }
    
    int64_t now = esp_timer_get_time() / 1000;
    if (now - s_last_activation_time < s_config.min_interval_ms) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_STATE;
    }
    
    gpio_set_level(s_gpio_num, 1);
    s_active = true;
    s_last_activation_time = now;
    
    esp_timer_start_once(s_pulse_timer, duration_ms * 1000);
    
    xSemaphoreGive(s_mutex);
    
    ESP_LOGI(TAG, "Activated relay for %" PRIu32 "ms", duration_ms);
    return ESP_OK;
}

esp_err_t relay_set_config(const relay_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    memcpy(&s_config, config, sizeof(relay_config_t));
    xSemaphoreGive(s_mutex);
    
    return ESP_OK;
}

esp_err_t relay_get_config(relay_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    memcpy(config, &s_config, sizeof(relay_config_t));
    xSemaphoreGive(s_mutex);
    
    return ESP_OK;
}

bool relay_is_active(void)
{
    bool active;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    active = s_active;
    xSemaphoreGive(s_mutex);
    return active;
}

esp_err_t relay_register_callback(relay_callback_t callback)
{
    if (!callback) {
        return ESP_ERR_INVALID_ARG;
    }
    s_callback = callback;
    return ESP_OK;
}