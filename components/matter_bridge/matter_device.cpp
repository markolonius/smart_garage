#include "matter_device.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "garage_door_control.h"
#include "reed_switch.h"

#define TAG "matter_device"

/* Matter node placeholder - will be implemented with ESP-Matter SDK */
static uint16_t window_covering_endpoint_id = 1;

/* Garage door state tracking */
static uint8_t current_position_percentage = 0;   /* 0 = closed, 100 = open */
static uint8_t operational_status = 0x00;        /* Operational: 0x00 = Stall */

/* Event group for door state changes */
static EventGroupHandle_t matter_event_group = NULL;
static const uint8_t MATTER_DOOR_STATE_CHANGED_BIT = BIT0;

/* Matter task handle */
static TaskHandle_t matter_task_handle = NULL;
static bool matter_running = false;

/* Forward declarations */
static void garage_door_command_callback(door_state_t state, void *priv_data);
static void matter_task(void *pvParameters);

/* Garage door state callback */
static void garage_door_command_callback(door_state_t state, void *priv_data)
{
    ESP_LOGI(TAG, "Garage door state: %s", garage_door_state_to_string(state));

    /* Update Matter attributes based on door state */
    uint8_t new_position = 0;
    uint8_t new_status = 0x00; /* Stall */

    switch (state) {
        case DOOR_STATE_OPEN:
            new_position = 100;
            new_status = 0x02; /* Operational */
            break;

        case DOOR_STATE_CLOSED:
            new_position = 0;
            new_status = 0x02; /* Operational */
            break;

        case DOOR_STATE_OPENING:
            new_position = current_position_percentage; /* Keep current */
            new_status = 0x04; /* Opening */
            break;

        case DOOR_STATE_CLOSING:
            new_position = current_position_percentage; /* Keep current */
            new_status = 0x05; /* Closing */
            break;

        case DOOR_STATE_STOPPED:
        case DOOR_STATE_ERROR:
            new_position = current_position_percentage; /* Keep current */
            new_status = 0x00; /* Stall */
            break;

        default:
            ESP_LOGW(TAG, "Unknown door state: %d", state);
            return;
    }

    /* Update current position if door reached end state */
    if (state == DOOR_STATE_OPEN || state == DOOR_STATE_CLOSED) {
        current_position_percentage = new_position;
    }

    ESP_LOGI(TAG, "Position: %u%%, Status: 0x%02x", new_position, new_status);

    /* TODO: When ESP-Matter is properly configured, update Matter attributes here:
     * - WindowCovering::CurrentPositionLiftPercentage100th
     * - WindowCovering::OperationalStatus
     */

    /* Notify event group */
    if (matter_event_group != NULL) {
        xEventGroupSetBits(matter_event_group, MATTER_DOOR_STATE_CHANGED_BIT);
    }
}

/* Matter task - will handle Matter communication when SDK is available */
static void matter_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Matter task started");

    while (matter_running) {
        /* Wait for door state changes */
        EventBits_t bits = xEventGroupWaitBits(matter_event_group,
                                               MATTER_DOOR_STATE_CHANGED_BIT,
                                               pdFALSE,
                                               pdMS_TO_TICKS(100));

        if (bits & MATTER_DOOR_STATE_CHANGED_BIT) {
            ESP_LOGD(TAG, "Door state changed, would update Matter attributes");

            /* TODO: Implement Matter attribute update when SDK is available:
             * esp_matter_attribute_update(window_covering_endpoint_id,
             *                                WindowCovering::Id,
             *                                WindowCovering::Attributes::CurrentPositionLiftPercentage100th::Id,
             *                                &position_val);
             */
        }
    }

    ESP_LOGI(TAG, "Matter task stopped");
    vTaskDelete(NULL);
}

/* Initialize Matter device */
esp_err_t matter_device_init(void)
{
    esp_err_t err = ESP_OK;

    ESP_LOGW(TAG, "Matter integration in stub mode - ESP-Matter SDK needs proper configuration");

    /* Create event group */
    matter_event_group = xEventGroupCreate();
    if (matter_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    /* Register garage door state callback */
    err = garage_door_register_state_callback(garage_door_command_callback);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to register door state callback: %s", esp_err_to_name(err));
        /* Continue anyway - we'll poll state in main loop */
    }

    /* Initialize current position from garage door state */
    door_state_t initial_state = garage_door_get_state();
    current_position_percentage = (initial_state == DOOR_STATE_OPEN) ? 100 : 0;

    /* Create Matter task */
    BaseType_t ret = xTaskCreate(matter_task,
                                "matter_task",
                                4096,
                                NULL,
                                2,
                                &matter_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Matter task");
        vEventGroupDelete(matter_event_group);
        return ESP_FAIL;
    }

    matter_running = true;

    ESP_LOGI(TAG, "Matter device initialized in stub mode");
    ESP_LOGI(TAG, "To enable full Matter functionality:");
    ESP_LOGI(TAG, "1. Configure ESP-Matter SDK in project");
    ESP_LOGI(TAG, "2. Add esp_matter component to CMakeLists.txt");
    ESP_LOGI(TAG, "3. Replace stub with actual ESP-Matter implementation");

    return ESP_OK;
}

/* Deinitialize Matter device */
esp_err_t matter_device_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing Matter device");

    /* Stop Matter task */
    matter_running = false;
    if (matter_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100)); /* Give task time to exit */
        matter_task_handle = NULL;
    }

    /* Delete event group */
    if (matter_event_group != NULL) {
        vEventGroupDelete(matter_event_group);
        matter_event_group = NULL;
    }

    ESP_LOGI(TAG, "Matter device deinitialized");
    return ESP_OK;
}

/* Update door state (called by main application) */
void matter_device_update_door_state(uint32_t position, bool is_moving)
{
    /* Update position attribute */
    current_position_percentage = (uint8_t)position;

    ESP_LOGI(TAG, "Door state: position=%" PRIu32 ", moving=%d", position, is_moving);

    /* TODO: When ESP-Matter is properly configured, update Matter attributes here:
     * - WindowCovering::CurrentPositionLiftPercentage100th
     * - WindowCovering::OperationalStatus
     */

    ESP_LOGD(TAG, "Matter attribute update would go here");
}
