idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES app_utils nvs_flash tuya_ble_transport tuya_storage
)
