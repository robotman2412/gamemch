
// This file contains a simple hello world app which you can base you own apps on.

#include "main.h"
#include "graphics.h"
#include "esp_timer.h"

pax_buf_t buf;
xQueueHandle buttonQueue;
Player *localPlayer;
Player *companion;

static const char *TAG = "main";

// Next time at which to broadcast info.
static uint64_t nextInfoBroadcast = 0;
// Interval between info broadcasts.
static uint64_t broadcastInterval = 3000;
// Current time.
static uint64_t now = 0;

// Updates the screen with the last drawing.
extern "C" void disp_flush() {
    ili9341_write(get_ili9341(), buf.buf_8bpp);
}

// Exits the app, returning to the launcher.
extern "C" void exit_to_launcher() {
    REG_WRITE(RTC_CNTL_STORE0_REG, 0);
    esp_restart();
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "Initialising hardware...");
    
    // Init the screen, the I2C and the SPI busses.
    bsp_init();
    // Init the RP2040 (responsible for buttons among other important things).
    bsp_rp2040_init();
    // This queue is used to await button presses.
    buttonQueue = get_rp2040()->queue;
    
    ESP_LOGI(TAG, "Initialising software...");
    
    // Init graphics for the screen.
    pax_buf_init(&buf, NULL, 320, 240, PAX_BUF_16_565RGB);
    // Init other graphics things.
    graphics_init();
    // Init NVS.
    nvs_flash_init();
    // Init (but not connect to) WiFi.
    wifi_init();
    
    ESP_LOGI(TAG, "Initialising radio...");
    
    // Start tasks.
    connection_start();
    espnow_start();
    
    // Load local player.
    localPlayer = loadFromNvs();
    
    while (1) {
        // Unset thel companion.
        for (int i = 0; i < connections.size(); i++) {
            if (connections[i]->status == Connection::OPEN) {
                if (!companion) {
                    connections[i]->send("info", "blob");
                }
                companion = connections[i]->player;
            }
        }
        
        graphics_task();
        
        now = esp_timer_get_time() / 1000;
        
        if (now >= nextInfoBroadcast) {
            // Broadcast info.
            broadcastInfo();
            nextInfoBroadcast = now + broadcastInterval;
        }
        
        // Await any button press or the next broadcast time.
        rp2040_input_message_t message;
        xQueueReceive(buttonQueue, &message, 1);
        
        // Is the home button currently pressed?
        if (message.input == RP2040_INPUT_BUTTON_HOME && message.state) {
            // If home is pressed, exit to launcher.
            exit_to_launcher();
        } else if (message.input == RP2040_INPUT_JOYSTICK_UP && message.state) {
            // Increment our score.
            localPlayer->addScore(1);
        } else if (message.input == RP2040_INPUT_JOYSTICK_DOWN && message.state) {
            // Decrement our score.
            localPlayer->addScore(-1);
        }
    }
}



// Broadcast info obout ourselves.
void broadcastInfo() {
    // Advertise our presence by broadcasting our nickname and score.
    espnow_broadcast("nick",  localPlayer->getNick());
    espnow_broadcast_num("score", localPlayer->getScore());
}



// Data event handling.
void mainDataCallback(Connection *from, const char *type, const char *data) {
    ESP_LOGI(TAG, "Message from %s: %s: %s", from->peer, type, data);
}

// Status event handling.
void mainStatusCallback(Connection *from) {
    ESP_LOGI(TAG, "Connection status for %s changed to %s", from->peer, from->statusToName());
}
