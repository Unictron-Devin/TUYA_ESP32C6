#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_log.h"

static const char *TAG = "TUYA_ESP32C6";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C6 Tuya Application Started");

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

    /* Application Loop */
    while (1)
    {
        ESP_LOGI(TAG, "Application is running...");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
