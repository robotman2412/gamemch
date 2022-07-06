
#include "espnowwrapper.h"
#include "esp_now.h"
#include "string.h"
#include <map>
#include "connection.h"
#include "main.h"
#include "esp_wifi.h"

static const char *TAG = "esp_now";

typedef union {
    uint8_t addr[6];
    uint64_t pack;
} mac_t;
std::map<uint64_t, Connection *> espnowConnMap;
static Connection *broadcaster;

static const uint8_t magic[] = {
    0xde, 0xad, 0xbe, 0xef,
    0x0a, 0x0d, 0xfe, 0xca
};
static const uint8_t broadcast_mac[] = {0xff,0xff,0xff,0xff,0xff,0xff,};

void espnow_start() {
    // Initialise WiFi AP.
    wifi_config_t wifi_config = {0};
    wifi_config.ap.authmode       = WIFI_AUTH_OPEN;
    wifi_config.ap.channel        = 1;
    strcpy((char *) wifi_config.ap.ssid, "ESPtestOF");
    wifi_config.ap.ssid_len       = 0;
    wifi_config.ap.ssid_hidden    = 1;
    wifi_config.ap.max_connection = 1;
    
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    
    // Initialise the API.
    esp_now_init();
    // Register callback for incoming data.
    esp_now_register_recv_cb(espnowRecvCallback);
    
    // Register broadcast thingy.
    esp_now_peer_info_t peer_info;
    peer_info.channel = 1;
    memcpy(peer_info.peer_addr, broadcast_mac, 6);
    peer_info.ifidx   = WIFI_IF_AP;
    peer_info.encrypt = false;
    esp_err_t res     = esp_now_add_peer(&peer_info);
    if (res) {
        ESP_LOGE(TAG, "Peer init error: %s", esp_err_to_name(res));
    }
    
    // Make dummy broadcast connection.
    mac_t converter = { .pack = 0 };
    memset(converter.addr, 0xff, 6);
    uint64_t mac_pak = converter.pack;
    Connection *conn = new Connection(espnowSendCallback);
    conn->peer = strdup("broadcast");
    espnowConnMap[mac_pak] = conn;
    broadcaster = conn;
}

void espnow_broadcast_raw(const char *msg) {
    char *buffer = new char[strlen(msg)+sizeof(magic)+1];
    memcpy(buffer, magic, sizeof(magic));
    memcpy(buffer+sizeof(magic), msg, strlen(msg)+1);
    
    esp_err_t res = esp_now_send(broadcast_mac, (const uint8_t *) buffer, strlen(buffer));
    if (res) {
        ESP_LOGE(TAG, "Broadcast error: %s", esp_err_to_name(res));
    }
    
    delete buffer;
}

void espnow_broadcast(const char *topic, const char *data) {
    broadcaster->send(topic, data);
}

void espnowRecvCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    if (data_len < sizeof(magic) || memcmp(data, magic, sizeof(magic))) {
        // This is not data for me.
        ESP_LOGI(TAG, "Magic is wrong.");
        return;
    }
    mac_t converter = { .pack = 0 };
    memcpy(converter.addr, mac_addr, 6);
    uint64_t mac_pak = converter.pack;
    
    Connection *conn = espnowConnMap[mac_pak];
    if (!conn) {
        ESP_LOGI(TAG, "Creating connection.");
        // Make a new one.
        char *id = (char *) malloc(18);
        snprintf(id, 18, "%016llx", mac_pak);
        conn = new Connection(espnowSendCallback);
        conn->dataCallbacks.push_back(mainDataCallback);
        conn->statusCallbacks.push_back(mainStatusCallback);
        conn->peer = id;
        espnowConnMap[mac_pak] = conn;
    }
    // Handle data on it.
    conn->onData(&data[sizeof(magic)], data_len - sizeof(magic));
}

void espnowSendCallback(Connection *from, const char *cstr) {
    espnow_broadcast_raw(cstr);
}