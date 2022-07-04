
// This file contains a simple hello world app which you can base you own apps on.

#include "main.h"
#include "btwrapper.h"

static pax_buf_t buf;
xQueueHandle buttonQueue;

static const char *TAG = "main";

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
    // Init NVS.
    nvs_flash_init();
    // Init (but not connect to) WiFi.
    wifi_init();
    
    ESP_LOGI(TAG, "Initialising radio...");
    pax_background(&buf, 0);
    pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "Connecting BlueTooth...");
    disp_flush();
    
    bt_start();
    
    while (1) {
        // Await any button press and do another cycle.
        // Structure used to receive data.
        rp2040_input_message_t message;
        // Await forever (because of portMAX_DELAY), a button press.
        xQueueReceive(buttonQueue, &message, portMAX_DELAY);
        
        // Is the home button currently pressed?
        if (message.input == RP2040_INPUT_BUTTON_HOME && message.state) {
            // If home is pressed, exit to launcher.
            exit_to_launcher();
        } else if (message.input == RP2040_INPUT_BUTTON_ACCEPT && message.state) {
            // Tell bluetooth to start a scan.
            bt_scan();
        }
    }
}

// Data event handling.
void mainDataCallback(Connection *from, const char *type, const char *data) {
    if (*type) ESP_LOGI(TAG, "Message from %s: %s: %s", from->peer, type, data);
    else ESP_LOGI(TAG, "Message from %s: %s", from->peer, data);
    from->send("This without topic");
    from->send("This", "With topic");
}

// Status event handling.
void mainStatusCallback(Connection *from) {
    ESP_LOGI(TAG, "Connection status for %s changed to %s", from->peer, from->statusToName());
}
