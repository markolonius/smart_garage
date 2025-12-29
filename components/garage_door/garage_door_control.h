#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    DOOR_STATE_CLOSED = 0,
    DOOR_STATE_OPENING = 1,
    DOOR_STATE_OPEN = 2,
    DOOR_STATE_CLOSING = 3,
    DOOR_STATE_STOPPED = 4,
    DOOR_STATE_UNKNOWN = 5
} door_state_t;

typedef void (*door_state_callback_t)(door_state_t state);

esp_err_t garage_door_init(void);
esp_err_t garage_door_deinit(void);
esp_err_t garage_door_open(void);
esp_err_t garage_door_close(void);
esp_err_t garage_door_stop(void);
door_state_t garage_door_get_state(void);
bool garage_door_is_moving(void);
esp_err_t garage_door_set_timeout(uint32_t timeout_ms);
esp_err_t garage_door_register_state_callback(door_state_callback_t callback);
const char *garage_door_state_to_string(door_state_t state);