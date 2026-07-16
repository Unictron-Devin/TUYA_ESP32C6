#include "tuya_ble_transport.h"

#include <string.h>

#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "os/os_mbuf.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

/* NimBLE UUID arrays use the Bluetooth little-endian byte order. */
static const ble_uuid128_t s_write_uuid = BLE_UUID128_INIT(
    0xD0, 0x07, 0x9B, 0x5F, 0x80, 0x00, 0x01, 0x80,
    0x01, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00);
static const ble_uuid128_t s_notify_uuid = BLE_UUID128_INIT(
    0xD0, 0x07, 0x9B, 0x5F, 0x80, 0x00, 0x01, 0x80,
    0x01, 0x10, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00);
static const ble_uuid128_t s_read_uuid = BLE_UUID128_INIT(
    0xD0, 0x07, 0x9B, 0x5F, 0x80, 0x00, 0x01, 0x80,
    0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
static const ble_uuid16_t s_service_uuid =
    BLE_UUID16_INIT(TUYA_BLE_SERVICE_UUID16);

static const char *TAG = "TUYA_BLE";
static tuya_ble_transport_config_t s_config;
static uint8_t s_own_addr_type;
static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t s_write_handle;
static uint16_t s_notify_handle;
static uint16_t s_read_handle;
static bool s_host_synced;
static bool s_notify_enabled;
static uint8_t s_read_value[TUYA_BLE_MAX_DATA_LEN];
static uint16_t s_read_value_len;

static int gap_event(struct ble_gap_event *event, void *arg);
static int gatt_access(uint16_t conn_handle, uint16_t attr_handle,
                       struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def s_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &s_service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &s_write_uuid.u,
                .access_cb = gatt_access,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                .val_handle = &s_write_handle,
            },
            {
                .uuid = &s_notify_uuid.u,
                .access_cb = gatt_access,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &s_notify_handle,
            },
            {
                .uuid = &s_read_uuid.u,
                .access_cb = gatt_access,
                .flags = BLE_GATT_CHR_F_READ,
                .val_handle = &s_read_handle,
            },
            { 0 },
        },
    },
    { 0 },
};

static esp_err_t nimble_rc_to_esp(int rc)
{
    return rc == 0 ? ESP_OK : ESP_FAIL;
}

static int gatt_access(uint16_t conn_handle, uint16_t attr_handle,
                       struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle;
    (void)arg;

    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR &&
        attr_handle == s_write_handle) {
        uint8_t buffer[TUYA_BLE_MAX_DATA_LEN];
        uint16_t len = 0;
        int rc = ble_hs_mbuf_to_flat(ctxt->om, buffer, sizeof(buffer), &len);
        if (rc != 0) {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        ESP_LOGI(TAG, "RX %u byte(s)", (unsigned)len);
        if (s_config.on_rx != NULL) {
            s_config.on_rx(buffer, len, s_config.callback_context);
        }
        return 0;
    }

    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR &&
        attr_handle == s_read_handle) {
        int rc = os_mbuf_append(ctxt->om, s_read_value, s_read_value_len);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    return BLE_ATT_ERR_UNLIKELY;
}

static void advertise(void)
{
    struct ble_hs_adv_fields adv_fields = {0};
    struct ble_hs_adv_fields rsp_fields = {0};
    struct ble_gap_adv_params params = {0};
    const char *name = ble_svc_gap_device_name();
    int rc;

    if (!s_host_synced || ble_gap_adv_active()) {
        return;
    }

    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv_fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(TUYA_BLE_SERVICE_UUID16),
    };
    adv_fields.num_uuids16 = 1;
    adv_fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&adv_fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Cannot set advertising data: rc=%d", rc);
        return;
    }

    rsp_fields.name = (uint8_t *)name;
    rsp_fields.name_len = strlen(name);
    rsp_fields.name_is_complete = 1;
    rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Cannot set scan response: rc=%d", rc);
        return;
    }

    params.conn_mode = BLE_GAP_CONN_MODE_UND;
    params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    params.itvl_min = 48; /* 30 ms, unit is 0.625 ms. */
    params.itvl_max = 96; /* 60 ms. */

    rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER,
                           &params, gap_event, NULL);
    if (rc == 0) {
        ESP_LOGI(TAG, "Advertising started: name=%s service=0xFD50", name);
    } else {
        ESP_LOGE(TAG, "Cannot start advertising: rc=%d", rc);
    }
}

static int gap_event(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            s_conn_handle = event->connect.conn_handle;
            ESP_LOGI(TAG, "Connected: handle=%u", s_conn_handle);
            if (s_config.on_connection != NULL) {
                s_config.on_connection(true, s_conn_handle,
                                       s_config.callback_context);
            }
        } else {
            ESP_LOGW(TAG, "Connection failed: status=%d", event->connect.status);
            advertise();
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Disconnected: reason=%d", event->disconnect.reason);
        if (s_config.on_connection != NULL) {
            s_config.on_connection(false, s_conn_handle,
                                   s_config.callback_context);
        }
        s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_notify_enabled = false;
        advertise();
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        advertise();
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        if (event->subscribe.attr_handle == s_notify_handle) {
            s_notify_enabled = event->subscribe.cur_notify != 0;
            ESP_LOGI(TAG, "Notify %s",
                     s_notify_enabled ? "enabled" : "disabled");
        }
        return 0;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "MTU updated: %u", event->mtu.value);
        return 0;

    case BLE_GAP_EVENT_NOTIFY_TX:
        if (event->notify_tx.status != 0) {
            ESP_LOGW(TAG, "Notify failed: status=%d", event->notify_tx.status);
        }
        return 0;

    default:
        return 0;
    }
}

static void on_reset(int reason)
{
    s_host_synced = false;
    s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
    s_notify_enabled = false;
    ESP_LOGE(TAG, "NimBLE reset: reason=%d", reason);
}

static void on_sync(void)
{
    int rc = ble_hs_util_ensure_addr(0);
    if (rc != 0) {
        ESP_LOGE(TAG, "No usable BLE identity address: rc=%d", rc);
        return;
    }

    rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "Cannot infer BLE address type: rc=%d", rc);
        return;
    }

    s_host_synced = true;
    advertise();
}

static void host_task(void *param)
{
    (void)param;
    ESP_LOGI(TAG, "NimBLE host task started");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t tuya_ble_transport_init(const tuya_ble_transport_config_t *config)
{
    if (config == NULL || config->device_name == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    s_config = *config;
    s_conn_handle = BLE_HS_CONN_HANDLE_NONE;

    esp_err_t err = nimble_port_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nimble_port_init failed: %s", esp_err_to_name(err));
        return err;
    }

    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb = on_sync;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    int rc = ble_svc_gap_device_name_set(config->device_name);
    if (rc != 0) {
        return ESP_FAIL;
    }

    rc = ble_gatts_count_cfg(s_services);
    if (rc == 0) {
        rc = ble_gatts_add_svcs(s_services);
    }
    if (rc != 0) {
        ESP_LOGE(TAG, "Cannot register Tuya GATT service: rc=%d", rc);
        return ESP_FAIL;
    }

    nimble_port_freertos_init(host_task);
    return ESP_OK;
}

esp_err_t tuya_ble_transport_start_advertising(void)
{
    if (!s_host_synced) {
        return ESP_ERR_INVALID_STATE;
    }
    advertise();
    return ble_gap_adv_active() ? ESP_OK : ESP_FAIL;
}

esp_err_t tuya_ble_transport_stop_advertising(void)
{
    if (!ble_gap_adv_active()) {
        return ESP_OK;
    }
    return nimble_rc_to_esp(ble_gap_adv_stop());
}

esp_err_t tuya_ble_transport_notify(const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0 || len > TUYA_BLE_MAX_DATA_LEN) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_conn_handle == BLE_HS_CONN_HANDLE_NONE || !s_notify_enabled) {
        return ESP_ERR_INVALID_STATE;
    }

    struct os_mbuf *om = ble_hs_mbuf_from_flat(data, len);
    if (om == NULL) {
        return ESP_ERR_NO_MEM;
    }

    int rc = ble_gatts_notify_custom(s_conn_handle, s_notify_handle, om);
    return nimble_rc_to_esp(rc);
}

esp_err_t tuya_ble_transport_set_read_value(const uint8_t *data, size_t len)
{
    if ((data == NULL && len != 0) || len > sizeof(s_read_value)) {
        return ESP_ERR_INVALID_ARG;
    }

    if (len != 0) {
        memcpy(s_read_value, data, len);
    }
    s_read_value_len = len;
    return ESP_OK;
}

bool tuya_ble_transport_is_connected(void)
{
    return s_conn_handle != BLE_HS_CONN_HANDLE_NONE;
}

bool tuya_ble_transport_is_notify_enabled(void)
{
    return s_notify_enabled;
}

uint16_t tuya_ble_transport_connection_handle(void)
{
    return s_conn_handle;
}
