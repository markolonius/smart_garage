idf_component_register(
    SRCS "reed_switch.c" "relay_control.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES "driver" "gpio"
)