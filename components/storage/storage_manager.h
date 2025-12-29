#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "esp_err.h"

#define STORAGE_NAMESPACE "garage_door"

typedef struct {
    uint32_t reed_closed_pin;
    uint32_t reed_open_pin;
    uint32_t relay_pin;
} storage_gpio_config_t;

typedef struct {
    uint32_t pulse_duration_ms;
    uint32_t max_pulse_duration_ms;
    uint32_t min_interval_ms;
} storage_relay_config_t;

typedef enum {
    EVENT_TYPE_DOOR_OPEN = 0,
    EVENT_TYPE_DOOR_CLOSED = 1,
    EVENT_TYPE_TIMEOUT = 2,
    EVENT_TYPE_OBSTRUCTION = 3,
    EVENT_TYPE_COMMISSION = 4,
    EVENT_TYPE_ERROR = 5
} event_type_t;

typedef struct {
    event_type_t type;
    uint32_t timestamp;
    int32_t value;
} event_log_t;

esp_err_t storage_init(void);
esp_err_t storage_save_gpio_config(const storage_gpio_config_t *config);
esp_err_t storage_load_gpio_config(storage_gpio_config_t *config);
esp_err_t storage_save_relay_config(const storage_relay_config_t *config);
esp_err_t storage_load_relay_config(storage_relay_config_t *config);
esp_err_t storage_save_door_state(uint32_t state);
esp_err_t storage_load_door_state(uint32_t *state);
esp_err_t storage_log_event(event_type_t type, int32_t value);
esp_err_t storage_get_logs(event_log_t *logs, size_t max_count, size_t *actual_count);
esp_err_t storage_factory_reset(void);