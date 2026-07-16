idf_component_register(
    SRCS "tuya_ble_transport.c"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash
)
