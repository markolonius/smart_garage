#pragma once

#include <stdint.h>
#include "driver/gpio.h"

typedef struct {
    uint32_t pulse_duration_ms;
    uint32_t max_pulse_duration_ms;
    uint32_t min_interval_ms;
} relay_config_t;

typedef void (*relay_callback_t)(void);

esp_err_t relay_init(gpio_num_t gpio_num);
esp_err_t relay_deinit(void);
esp_err_t relay_activate(void);
esp_err_t relay_activate_pulse(uint32_t duration_ms);
esp_err_t relay_set_config(const relay_config_t *config);
esp_err_t relay_get_config(relay_config_t *config);
bool relay_is_active(void);
esp_err_t relay_register_callback(relay_callback_t callback);