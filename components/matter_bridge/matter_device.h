#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t matter_device_init(void);
esp_err_t matter_device_deinit(void);
void matter_device_update_door_state(uint32_t position, bool is_moving);

#ifdef __cplusplus
}
#endif