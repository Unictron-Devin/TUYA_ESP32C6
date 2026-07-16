#include "tuya_storage.h"

#include <string.h>

#include "esp_check.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#define TUYA_NVS_PARTITION "tuya_data"
#define TUYA_NVS_NAMESPACE "credentials"

static const char *TAG = "TUYA_STORAGE";

static esp_err_t read_string(nvs_handle_t handle, const char *key,
                             char *value, size_t capacity)
{
    size_t required = capacity;
    esp_err_t err = nvs_get_str(handle, key, value, &required);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        value[0] = '\0';
    }
    return err;
}

esp_err_t tuya_storage_init(void)
{
    esp_err_t err = nvs_flash_init_partition(TUYA_NVS_PARTITION);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing incompatible Tuya NVS partition");
        ESP_RETURN_ON_ERROR(nvs_flash_erase_partition(TUYA_NVS_PARTITION),
                            TAG, "Cannot erase Tuya NVS partition");
        err = nvs_flash_init_partition(TUYA_NVS_PARTITION);
    }
    return err;
}

esp_err_t tuya_storage_load_credentials(tuya_credentials_t *credentials)
{
    if (credentials == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(credentials, 0, sizeof(*credentials));

    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(TUYA_NVS_PARTITION,
                                            TUYA_NVS_NAMESPACE,
                                            NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_ERR_NOT_FOUND;
    }
    if (err != ESP_OK) {
        return err;
    }

    err = read_string(handle, "product_id", credentials->product_id,
                      sizeof(credentials->product_id));
    if (err == ESP_OK) {
        err = read_string(handle, "uuid", credentials->uuid,
                          sizeof(credentials->uuid));
    }
    if (err == ESP_OK) {
        err = read_string(handle, "auth_key", credentials->auth_key,
                          sizeof(credentials->auth_key));
    }
    nvs_close(handle);
    return err;
}

esp_err_t tuya_storage_save_credentials(const tuya_credentials_t *credentials)
{
    if (credentials == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(TUYA_NVS_PARTITION,
                                            TUYA_NVS_NAMESPACE,
                                            NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_str(handle, "product_id", credentials->product_id);
    if (err == ESP_OK) {
        err = nvs_set_str(handle, "uuid", credentials->uuid);
    }
    if (err == ESP_OK) {
        err = nvs_set_str(handle, "auth_key", credentials->auth_key);
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

esp_err_t tuya_storage_clear_credentials(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(TUYA_NVS_PARTITION,
                                            TUYA_NVS_NAMESPACE,
                                            NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_erase_all(handle);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}
