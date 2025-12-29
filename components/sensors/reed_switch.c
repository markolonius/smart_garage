#include "reed_switch.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_intr_alloc.h"

#define DEBOUNCE_MS 50
#define TAG "reed_switch"

static reed_switch_config_t s_config;
static bool s_initialized = false;
static reed_switch_callback_t s_callback = NULL;
static volatile door_position_t s_current_position = DOOR_POSITION_UNKNOWN;
static esp_timer_handle_t s_debounce_timer = NULL;
static volatile bool s_debounce_pending = false;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    if (!s_debounce_pending) {
        s_debounce_pending = true;
        if (s_debounce_timer) {
            esp_timer_start_once(s_debounce_timer, DEBOUNCE_MS * 1000);
        }
    }
}

static void debounce_timer_callback(void *arg)
{
    s_debounce_pending = false;
    door_position_t new_pos = reed_switch_get_position();
    
    if (new_pos != s_current_position) {
        s_current_position = new_pos;
        ESP_LOGI(TAG, "Position changed to %d", s_current_position);
        if (s_callback) {
            s_callback(s_current_position);
        }
    }
}

esp_err_t reed_switch_init(const reed_switch_config_t *config)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_config, config, sizeof(reed_switch_config_t));
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->reed_closed_pin) | (1ULL << config->reed_open_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    esp_timer_create_args_t timer_args = {
        .callback = debounce_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "debounce"
    };
    
    ret = esp_timer_create(&timer_args, &s_debounce_timer);
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        return ret;
    }
    
    gpio_isr_handler_add(config->reed_closed_pin, gpio_isr_handler, NULL);
    gpio_isr_handler_add(config->reed_open_pin, gpio_isr_handler, NULL);
    
    s_current_position = reed_switch_get_position();
    s_initialized = true;
    
    ESP_LOGI(TAG, "Initialized on pins %d (closed), %d (open)", config->reed_closed_pin, config->reed_open_pin);
    return ESP_OK;
}

esp_err_t reed_switch_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    gpio_isr_handler_remove(s_config.reed_closed_pin);
    gpio_isr_handler_remove(s_config.reed_open_pin);
    
    if (s_debounce_timer) {
        esp_timer_stop(s_debounce_timer);
        esp_timer_delete(s_debounce_timer);
        s_debounce_timer = NULL;
    }
    
    s_initialized = false;
    s_callback = NULL;
    return ESP_OK;
}

door_position_t reed_switch_get_position(void)
{
    if (!s_initialized) {
        return DOOR_POSITION_UNKNOWN;
    }
    
    bool closed = (gpio_get_level(s_config.reed_closed_pin) == 0);
    bool open = (gpio_get_level(s_config.reed_open_pin) == 0);
    
    if (closed && !open) {
        return DOOR_POSITION_CLOSED;
    } else if (open && !closed) {
        return DOOR_POSITION_OPEN;
    } else if (!closed && !open) {
        return DOOR_POSITION_BETWEEN;
    }
    
    return DOOR_POSITION_UNKNOWN;
}

bool reed_switch_is_closed(void)
{
    return reed_switch_get_position() == DOOR_POSITION_CLOSED;
}

bool reed_switch_is_open(void)
{
    return reed_switch_get_position() == DOOR_POSITION_OPEN;
}

esp_err_t reed_switch_register_callback(reed_switch_callback_t callback)
{
    if (!callback) {
        return ESP_ERR_INVALID_ARG;
    }
    s_callback = callback;
    return ESP_OK;
}

esp_err_t reed_switch_set_gpio_config(const reed_switch_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(&s_config, config, sizeof(reed_switch_config_t));
    return ESP_OK;
}