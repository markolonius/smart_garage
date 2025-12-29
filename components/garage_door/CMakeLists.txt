idf_component_register(
    SRCS "garage_door_control.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES "sensors" "storage"
)