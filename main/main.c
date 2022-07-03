
// This file contains a simple hello world app which you can base you own apps on.

#include "main.h"
#include "btwrapper.h"

static pax_buf_t buf;
xQueueHandle buttonQueue;

static const char *TAG = "mch2022-demo-app";

// Updates the screen with the last drawing.
void disp_flush() {
    ili9341_write(get_ili9341(), buf.buf);
}

// Exits the app, returning to the launcher.
void exit_to_launcher() {
    REG_WRITE(RTC_CNTL_STORE0_REG, 0);
    esp_restart();
}

void app_main() {
    // Init the screen, the I2C and the SPI busses.
    bsp_init();
    // Init the RP2040 (responsible for buttons among other important things).
    bsp_rp2040_init();
    // This queue is used to await button presses.
    buttonQueue = get_rp2040()->queue;
    
    // Init graphics for the screen.
    pax_buf_init(&buf, NULL, 320, 240, PAX_BUF_16_565RGB);
    
    // Init NVS.
    nvs_flash_init();
    
    // Init (but not connect to) WiFi.
    wifi_init();
    
    pax_background(&buf, 0);
    pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "Connecting BlueTooth...");
    disp_flush();
    
    bt_start();
    
    if (1) {
        pax_background(&buf, 0);
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "Connected!");
        disp_flush();
    } else {
        pax_background(&buf, 0);
        pax_draw_text(&buf, 0xffff0000, pax_font_saira_regular, 18, 5, 5, "Erreur");
        disp_flush();
    }
    
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
        }
    }
}
