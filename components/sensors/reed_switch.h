#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"

typedef struct {
    gpio_num_t reed_closed_pin;
    gpio_num_t reed_open_pin;
    gpio_num_t relay_pin;
} reed_switch_config_t;

typedef enum {
    DOOR_POSITION_UNKNOWN = 0,
    DOOR_POSITION_CLOSED = 1,
    DOOR_POSITION_OPEN = 2,
    DOOR_POSITION_BETWEEN = 3
} door_position_t;

typedef void (*reed_switch_callback_t)(door_position_t position);

esp_err_t reed_switch_init(const reed_switch_config_t *config);
esp_err_t reed_switch_deinit(void);
door_position_t reed_switch_get_position(void);
bool reed_switch_is_closed(void);
bool reed_switch_is_open(void);
esp_err_t reed_switch_register_callback(reed_switch_callback_t callback);
esp_err_t reed_switch_set_gpio_config(const reed_switch_config_t *config);