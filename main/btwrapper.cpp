
#include "btwrapper.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "connection.h"
#include <string.h>
#include <malloc.h>

// #define USE_BLE

static const char *TAG = "bluetooth";
#define SPP_SERVER_NAME "ESP32_SPP_SERVER"
#define DEV_NAME "ESP32 SPP test"

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

std::map<int, Connection*> connMap;

static char *bda2str(uint8_t * bda, char *str, size_t size) {
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
            Connection *conn;
        
        case ESP_SPP_INIT_EVT:
            if (param->init.status == ESP_SPP_SUCCESS) {
                ESP_LOGI(TAG, "SPP initialised");
                esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME);
            } else {
                ESP_LOGE(TAG, "SPP init error status: %d", param->init.status);
            }
            break;
        case ESP_SPP_DISCOVERY_COMP_EVT:
            ESP_LOGI(TAG, "SPP discovery complete");
            break;
        case ESP_SPP_OPEN_EVT:
            ESP_LOGI(TAG, "SPP opened");
            conn = new Connection(bluetoothSendCallback);
            conn->dataCallbacks.push_back(mainDataCallback);
            conn->statusCallbacks.push_back(mainStatusCallback);
            conn->peer = bda2str(param->open.rem_bda, (char*) malloc(64), 64);
            conn->setStatus(Connection::Status::OPEN);
            connMap[param->open.handle] = conn;
            break;
        case ESP_SPP_CLOSE_EVT:
            ESP_LOGI(TAG, "SPP closed");
            conn = connMap[param->close.handle];
            if (conn) {
                conn->setStatus(Connection::Status::CLOSED);
                connMap.erase(param->close.handle);
                delete conn;
            } else {
                ESP_LOGE(TAG, "No connection for handle %d!", param->close.handle);
            }
            break;
        case ESP_SPP_START_EVT:
            if (param->start.status == ESP_SPP_SUCCESS) {
                ESP_LOGI(TAG, "SPP server started");
                esp_bt_dev_set_device_name(DEV_NAME);
                esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            } else {
                ESP_LOGE(TAG, "SPP server start error, status: %d", param->start.status);
            }
            break;
        case ESP_SPP_CL_INIT_EVT:
            ESP_LOGI(TAG, "SPP Client initialised");
            break;
        case ESP_SPP_DATA_IND_EVT:
            conn = connMap[param->data_ind.handle];
            if (conn) {
                conn->onData(param->data_ind.data, param->data_ind.len);
            } else {
                ESP_LOGE(TAG, "No connection for handle %d!", param->close.handle);
            }
            break;
        case ESP_SPP_SRV_OPEN_EVT:
            ESP_LOGI(TAG, "SPP server opened");
            conn = new Connection(bluetoothSendCallback);
            conn->dataCallbacks.push_back(mainDataCallback);
            conn->statusCallbacks.push_back(mainStatusCallback);
            conn->peer = bda2str(param->srv_open.rem_bda, (char*) malloc(64), 64);
            conn->setStatus(Connection::Status::OPEN);
            connMap[param->srv_open.handle] = conn;
            break;
        case ESP_SPP_SRV_STOP_EVT:
            ESP_LOGI(TAG, "SPP server stop");
            break;
        case ESP_SPP_UNINIT_EVT:
            ESP_LOGI(TAG, "SPP deinit");
            break;
        default:
            break;
    }
}

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    char bda_str[18] = {0};

    switch (event) {
        case ESP_BT_GAP_AUTH_CMPL_EVT:{
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "authentication success: %s bda: %s ", param->auth_cmpl.device_name,
                        bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
            } else {
                ESP_LOGE(TAG, "authentication failed, status: %d", param->auth_cmpl.stat);
            }
            break;
        }
        case ESP_BT_GAP_PIN_REQ_EVT:{
            ESP_LOGI(TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit: %d", param->pin_req.min_16_digit);
            if (param->pin_req.min_16_digit) {
                ESP_LOGI(TAG, "Input pin code: 0000 0000 0000 0000");
                esp_bt_pin_code_t pin_code = {0};
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
            } else {
                ESP_LOGI(TAG, "Input pin code: 1234");
                esp_bt_pin_code_t pin_code;
                pin_code[0] = '1';
                pin_code[1] = '2';
                pin_code[2] = '3';
                pin_code[3] = '4';
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            }
            break;
        }
        case ESP_BT_GAP_DISC_RES_EVT:
            ESP_LOGI(TAG, "Found %s", bda2str(param->disc_res.bda, bda_str, sizeof(bda_str)));
            for (size_t i = 0; i < param->disc_res.num_prop; i++) {
                esp_bt_gap_dev_prop_t *prop = &param->disc_res.prop[i];
                ESP_LOGI(TAG, "  %d: %d byte(s)", prop->type, prop->len);
                if (prop->type == ESP_BT_GAP_DEV_PROP_BDNAME) {
                    char *buf = (char*) malloc(prop->len+1);
                    memcpy(buf, prop->val, prop->len);
                    buf[prop->len]=0;
                    ESP_LOGI(TAG, "  Name: %s", buf);
                    free((void*) buf);
                } else if (prop->type == ESP_BT_GAP_DEV_PROP_EIR) {
                    parseEir((uint8_t*) prop->val, prop->len);
                }
            }
            break;

#if (CONFIG_BT_SSP_ENABLED == true)
        case ESP_BT_GAP_CFM_REQ_EVT:
            ESP_LOGI(TAG, "Please compare the numeric value: %d", param->cfm_req.num_val);
            esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
            break;
        case ESP_BT_GAP_KEY_NOTIF_EVT:
            ESP_LOGI(TAG, "passkey:%d", param->key_notif.passkey);
            break;
        case ESP_BT_GAP_KEY_REQ_EVT:
            ESP_LOGI(TAG, "Please enter passkey.");
            break;
#endif
        default:
            break;
    }
    return;
}

void bt_start() {
    char bda_str[18] = {0};
    esp_err_t ret=0;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(TAG, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
        ESP_LOGE(TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);

    ESP_LOGI(TAG, "Own address: %s", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));
}

void bluetoothSendCallback(Connection *from, const char *cstr) {
    int handle;
    for (auto iter = connMap.begin(); iter != connMap.end(); iter ++) {
        if (iter->second == from) {
            handle = iter->first;
            
            esp_spp_write(handle, strlen(cstr), (uint8_t *) cstr);
            esp_spp_write(handle, 2, (uint8_t *) "\r\n");
            ESP_LOGI(TAG, "Message to %s: %s", from->peer, cstr);
            return;
        }
    }
    ESP_LOGE(TAG, "Unable to find handle to send to!");
}

void bt_scan() {
    esp_bt_inq_mode_t mode = ESP_BT_INQ_MODE_GENERAL_INQUIRY;
    esp_bt_gap_start_discovery(mode, 0x5, 0);
    ESP_LOGI(TAG, "Discovery started.");
}

void parseEir(uint8_t *data, size_t len) {
    // esp_bt_gap_resolve_eir_data(data, 0, &len);
}
