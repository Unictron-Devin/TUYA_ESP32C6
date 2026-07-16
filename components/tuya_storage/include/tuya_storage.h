#ifndef TUYA_STORAGE_H
#define TUYA_STORAGE_H

#include <stddef.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TUYA_PRODUCT_ID_MAX_LEN 32
#define TUYA_UUID_MAX_LEN       32
#define TUYA_AUTH_KEY_MAX_LEN   32

typedef struct {
    char product_id[TUYA_PRODUCT_ID_MAX_LEN + 1];
    char uuid[TUYA_UUID_MAX_LEN + 1];
    char auth_key[TUYA_AUTH_KEY_MAX_LEN + 1];
} tuya_credentials_t;

esp_err_t tuya_storage_init(void);
esp_err_t tuya_storage_load_credentials(tuya_credentials_t *credentials);
esp_err_t tuya_storage_save_credentials(const tuya_credentials_t *credentials);
esp_err_t tuya_storage_clear_credentials(void);

#ifdef __cplusplus
}
#endif

#endif
