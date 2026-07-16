#include "app_utils.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "APP_UTILS";

void app_utils_init(void)
{
    ESP_LOGI(TAG, "Application utilities initialized");
}

uint64_t app_utils_get_uptime_ms(void)
{
    // Returns tick count in milliseconds
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void app_utils_print_mem_stats(void)
{
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();

    ESP_LOGI(TAG, "Free heap: %lu bytes, Min free heap: %lu bytes",
             free_heap, min_free_heap);
}

uint32_t app_utils_get_free_heap(void)
{
    return esp_get_free_heap_size();
}
