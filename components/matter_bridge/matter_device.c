#include "matter_device.h"
#include "esp_log.h"

#define TAG "matter_device"

esp_err_t matter_device_init(void)
{
    ESP_LOGW(TAG, "Matter integration not yet implemented");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t matter_device_deinit(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}

void matter_device_update_door_state(uint32_t position, bool is_moving)
{
    ESP_LOGD(TAG, "Door state: position=%" PRIu32 ", moving=%d", position, is_moving);
}