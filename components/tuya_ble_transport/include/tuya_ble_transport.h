#ifndef TUYA_BLE_TRANSPORT_H
#define TUYA_BLE_TRANSPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TUYA_BLE_SERVICE_UUID16 0xFD50
#define TUYA_BLE_MAX_DATA_LEN   244

typedef void (*tuya_ble_rx_callback_t)(const uint8_t *data, size_t len,
                                      void *context);
typedef void (*tuya_ble_connection_callback_t)(bool connected,
                                              uint16_t conn_handle,
                                              void *context);

typedef struct {
    const char *device_name;
    tuya_ble_rx_callback_t on_rx;
    tuya_ble_connection_callback_t on_connection;
    void *callback_context;
} tuya_ble_transport_config_t;

/**
 * Initialize ESP-NimBLE and expose the Tuya FD50 GATT transport.
 *
 * The transport implements the UUID layout expected by Tuya BLE protocol v2:
 *  - FD50 service
 *  - 0001 write characteristic
 *  - 0002 notify characteristic
 *  - 0003 read characteristic
 */
esp_err_t tuya_ble_transport_init(const tuya_ble_transport_config_t *config);

/** Start connectable legacy advertising. */
esp_err_t tuya_ble_transport_start_advertising(void);

/** Stop advertising if it is active. */
esp_err_t tuya_ble_transport_stop_advertising(void);

/** Send protocol bytes through the Tuya notify characteristic. */
esp_err_t tuya_ble_transport_notify(const uint8_t *data, size_t len);

/** Update bytes returned from the Tuya read characteristic. */
esp_err_t tuya_ble_transport_set_read_value(const uint8_t *data, size_t len);

bool tuya_ble_transport_is_connected(void);
bool tuya_ble_transport_is_notify_enabled(void);
uint16_t tuya_ble_transport_connection_handle(void);

#ifdef __cplusplus
}
#endif

#endif
