#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "storage_manager.h"
#include "reed_switch.h"
#include "relay_control.h"
#include "garage_door_control.h"

#define TAG "app_main"

#define DEFAULT_REED_CLOSED_PIN GPIO_NUM_2
#define DEFAULT_REED_OPEN_PIN GPIO_NUM_3
#define DEFAULT_RELAY_PIN GPIO_NUM_4

static void door_state_callback(door_state_t state)
{
    ESP_LOGI(TAG, "Door state: %s", garage_door_state_to_string(state));
}

void app_main(void)
{
    ESP_LOGI(TAG, "Smart Garage Door Controller Starting");
    
    esp_err_t ret = storage_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize storage: %s", esp_err_to_name(ret));
        return;
    }
    
    storage_gpio_config_t gpio_config;
    ret = storage_load_gpio_config(&gpio_config);
    if (ret != ESP_OK || gpio_config.relay_pin == 0) {
        ESP_LOGW(TAG, "Using default GPIO configuration");
        gpio_config.reed_closed_pin = DEFAULT_REED_CLOSED_PIN;
        gpio_config.reed_open_pin = DEFAULT_REED_OPEN_PIN;
        gpio_config.relay_pin = DEFAULT_RELAY_PIN;
        storage_save_gpio_config(&gpio_config);
    }
    
    ESP_LOGI(TAG, "GPIO config: reed_closed=%" PRIu32 ", reed_open=%" PRIu32 ", relay=%" PRIu32,
             gpio_config.reed_closed_pin, gpio_config.reed_open_pin, gpio_config.relay_pin);
    
    reed_switch_config_t reed_config = {
        .reed_closed_pin = gpio_config.reed_closed_pin,
        .reed_open_pin = gpio_config.reed_open_pin,
        .relay_pin = gpio_config.relay_pin
    };
    
    ret = reed_switch_init(&reed_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize reed switches: %s", esp_err_to_name(ret));
        return;
    }
    
    storage_relay_config_t relay_config;
    ret = storage_load_relay_config(&relay_config);
    if (ret != ESP_OK || relay_config.pulse_duration_ms == 0) {
        ESP_LOGW(TAG, "Using default relay configuration");
        relay_config.pulse_duration_ms = 500;
        relay_config.max_pulse_duration_ms = 600;
        relay_config.min_interval_ms = 1000;
        storage_save_relay_config(&relay_config);
    }
    
    ret = relay_init(gpio_config.relay_pin);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize relay: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = relay_set_config(&relay_config);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set relay config: %s", esp_err_to_name(ret));
    }
    
    ret = garage_door_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize garage door: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = garage_door_register_state_callback(door_state_callback);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to register state callback: %s", esp_err_to_name(ret));
    }
    
    ESP_LOGI(TAG, "Initialization complete. Door state: %s", garage_door_state_to_string(garage_door_get_state()));
    
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Door state: %s, Position: %d", 
                 garage_door_state_to_string(garage_door_get_state()),
                 reed_switch_get_position());
    }
}