#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "app_utils.h"
#include "tuya_ble_transport.h"
#include "tuya_storage.h"

static const char *TAG = "TUYA_ESP32C6";

static void on_tuya_rx(const uint8_t *data, size_t len, void *context)
{
    (void)context;
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, data, len, ESP_LOG_INFO);

    /* Transport validation: echo received bytes after the client subscribes.
     * Replace this call with tuya_ble_event_process() when a RISC-V compatible
     * Tuya secure/protocol library is available. */
    esp_err_t err = tuya_ble_transport_notify(data, len);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "Echo notify failed: %s", esp_err_to_name(err));
    }
}

static void on_tuya_connection(bool connected, uint16_t conn_handle,
                               void *context)
{
    (void)context;
    ESP_LOGI(TAG, "Tuya BLE %s (handle=%u)",
             connected ? "connected" : "disconnected", conn_handle);
}

static esp_err_t init_default_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "Cannot erase NVS");
        err = nvs_flash_init();
    }
    return err;
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C6 Tuya Application Started");

    ESP_ERROR_CHECK(init_default_nvs());
    ESP_ERROR_CHECK(tuya_storage_init());
    app_utils_init();

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG, "This is %s chip with %d CPU core(s), WiFi%s%s%s",
             CONFIG_IDF_TARGET,
             chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
             (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4" : "");

    ESP_LOGI(TAG, "silicon revision v%d", chip_info.revision);

    tuya_credentials_t credentials;
    esp_err_t err = tuya_storage_load_credentials(&credentials);
    if (err == ESP_ERR_NOT_FOUND || credentials.product_id[0] == '\0') {
        ESP_LOGW(TAG, "Tuya credentials are not provisioned yet");
    } else if (err == ESP_OK) {
        /* Never log UUID or auth_key. */
        ESP_LOGI(TAG, "Tuya product ID loaded: %s", credentials.product_id);
    } else {
        ESP_LOGW(TAG, "Cannot read Tuya credentials: %s",
                 esp_err_to_name(err));
    }

    const uint8_t read_value[] = { 'E', 'S', 'P', '3', '2', '-', 'C', '6' };
    const tuya_ble_transport_config_t ble_config = {
        .device_name = "TUYA_ESP32C6",
        .on_rx = on_tuya_rx,
        .on_connection = on_tuya_connection,
    };
    ESP_ERROR_CHECK(tuya_ble_transport_set_read_value(read_value,
                                                       sizeof(read_value)));
    ESP_ERROR_CHECK(tuya_ble_transport_init(&ble_config));

    /* Application Loop */
    while (1)
    {
        ESP_LOGI(TAG, "running, free heap=%" PRIu32 ", BLE=%s, notify=%s",
                 app_utils_get_free_heap(),
                 tuya_ble_transport_is_connected() ? "connected" : "idle",
                 tuya_ble_transport_is_notify_enabled() ? "on" : "off");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
