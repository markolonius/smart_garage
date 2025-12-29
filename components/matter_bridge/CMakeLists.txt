idf_component_register(
    SRCS "matter_device.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES "esp_matter" "wifi"
)