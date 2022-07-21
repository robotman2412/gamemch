
// This file contains a simple hello world app which you can base you own apps on.

#include "main.h"
#include "graphics.h"
#include "esp_timer.h"
#include "string.h"

pax_buf_t buf;
xQueueHandle buttonQueue;
Player *localPlayer;
Player *companion;
bool hasCompanion;
bool companionAgrees;
Screen currentScreen;
std::vector<int> companionList;
int companionListIndex;

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
    pax_enable_multicore(1);
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
    currentScreen = Screen::HOME;
    
    while (1) {
        espnow_run_callbacks();
        if (currentScreen == Screen::COMP_SELECT) {
            refreshCompanionList();
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
        if (xQueueReceive(buttonQueue, &message, 1)) {
            
            if (message.input == RP2040_INPUT_BUTTON_HOME && message.state) {
                // If home is pressed, exit to launcher.
                exit_to_launcher();
                
            } else if (message.input == RP2040_INPUT_JOYSTICK_UP && message.state) {
                if (currentScreen == Screen::HOME) {
                    // Debug: Previous set.
                    if (debugAttrIndex > 0) debugAttrIndex --;
                }
                companionListIndex --;
                
            } else if (message.input == RP2040_INPUT_JOYSTICK_DOWN && message.state) {
                if (currentScreen == Screen::HOME) {
                    // Debug: Next set.
                    if (debugAttrIndex < attributeSets.size() - 1) debugAttrIndex ++;
                }
                companionListIndex ++;
                
            } else if (message.input == RP2040_INPUT_BUTTON_ACCEPT && message.state) {
                if (currentScreen == Screen::HOME) {
                    // Debug: Toggle set.
                    localPlayer->blob->toggleSet(attributeSets[debugAttrIndex]);
                    localPlayer->blob->applyAttributes();
                    if (companion) {
                        localPlayer->blob->send(companion->connection);
                    }
                }
                if (currentScreen == Screen::COMP_SELECT) {
                    // Ask thel compañor.
                    currentScreen = Screen::COMP_AWAIT;
                    askCompanion(connections[companionList[companionListIndex]]);
                }
                
            } else if (message.input == RP2040_INPUT_BUTTON_SELECT && message.state && !hasCompanion) {
                if (nearby == 1) {
                    // Just ask this one directly.
                    currentScreen = Screen::COMP_AWAIT;
                    askCompanion(firstNearby);
                } else if (nearby) {
                    // Go to the selection menu.
                    currentScreen = Screen::COMP_SELECT;
                }
                
            } else if (message.input == RP2040_INPUT_BUTTON_BACK && message.state) {
                if (hasCompanion || currentScreen == Screen::COMP_AWAIT) {
                    // Manually leave.
                    companion->connection->send("info", "leave");
                    companion->askedUsOut = false;
                    setCompanion(NULL, false);
                    currentScreen = Screen::HOME;
                }
                if (currentScreen == Screen::COMP_SELECT) {
                    // Cancel selecting.
                    currentScreen = Screen::HOME;
                }
                
            } else if (message.input == RP2040_INPUT_JOYSTICK_PRESS && message.state) {
                // Debug: Change blob with maximum random.
                localPlayer->blob->redoAttributes();
                if (companion) {
                    localPlayer->blob->send(companion->connection);
                }
                
            } else if (message.input == RP2040_INPUT_BUTTON_MENU && message.state) {
                if (hasCompanion) {
                    // Debug: Do the mutator.
                    ESP_LOGI(TAG, "Mutating with companion");
                    localPlayer->blob->mutate(companion->blob);
                    localPlayer->blob->send(companion->connection);
                } else {
                    // Debug: Randomise again the attributes.
                    ESP_LOGI(TAG, "Randomising");
                    localPlayer->blob->attributes.clear();
                    localPlayer->blob->initialRandomise();
                }
                
            }
        }
    }
}



// Broadcast info about ourselves.
void broadcastInfo() {
    // Advertise our presence by broadcasting our nickname and score.
    espnow_broadcast("nick",              localPlayer->getNick());
    espnow_broadcast_num("blob_body_col", localPlayer->blob->bodyColor);
}

// Ask a connection as companion.
void askCompanion(Connection *to) {
    // Send our name to this player in case they didn't get the broadcast.
    to->send   ("nick",  localPlayer->getNick());
    to->sendNum("score", localPlayer->getScore());
    
    if (!to->player->askedUsOut) {
        // They didn't ask us yet, so ask them first.
        setCompanion(to, false);
        to->send    ("info",  "request_companion");
        currentScreen = Screen::COMP_AWAIT;
        ESP_LOGI(TAG, "Out request %s", to->player->getNick());
    } else {
        // They already asked us, so accept.
        setCompanion(to, true);
        to->send    ("info",  "accept_companion");
        currentScreen = Screen::HOME;
        ESP_LOGI(TAG, "Out accept %s", to->player->getNick());
    }
}

// Sets the companion.
void setCompanion(Connection *to, bool agrees) {
    if (to) {
        companion = to->player;
        companionAgrees = agrees;
        if (agrees) {
            // Request their blob.
            to->send("info", "blob");
            // And we're no longer awaiting confirmation.
            currentScreen = Screen::HOME;
        }
        hasCompanion = true;
    } else {
        hasCompanion = false;
    }
}

// Refreshes the list of candidate companions.
void refreshCompanionList() {
    // Hacky thing for now.
    companionList.clear();
    for (int i = 0; i < connections.size(); i++) {
        Connection *conn = connections[i];
        if (conn->status == Connection::OPEN) {
            companionList.push_back(i);
        }
    }
    if (companionListIndex >= companionList.size()) {
        companionListIndex = 0;
    } else if (companionListIndex < 0) {
        companionListIndex = companionList.size() - 1;
    }
}



// Data event handling.
void mainDataCallback(Connection *from, const char *type, const char *data) {
    // ESP_LOGI(TAG, "Message from %s: %s: %s", from->peer, type, data);
    
    if (!strcmp(type, "info")) {
        if (!strcmp(data, "accept_companion")) {
            // An accepted companion request.
            if (companion == from->player) {
                // We asked them, set the accepted flag.
                setCompanion(from, true);
                ESP_LOGI(TAG, "In accept %s", from->player->getNick());
            } else {
                // We didn't ask them, clear up the miscommunication.
                from->send("info", "leave");
                ESP_LOGI(TAG, "Miscomm %s", from->player->getNick());
            }
            
        } else if (!strcmp(data, "request_companion")) {
            // They have officially asked us out.
            from->player->askedUsOut = true;
            ESP_LOGI(TAG, "In request %s", from->player->getNick());
            
            if (hasCompanion && companion == from->player) {
                // We asked them, so accept.
                from->send("info", "accept_companion");
                setCompanion(from, true);
                ESP_LOGI(TAG, "Out accept %s", from->player->getNick());
            }
            
        } else if (!strcmp(data, "leave") && hasCompanion) {
            // Our companion leaves.
            if (companion == from->player) {
                setCompanion(NULL, false);
                ESP_LOGI(TAG, "In leave %s", from->player->getNick());
            }
            from->player->askedUsOut = false;
        }
    }
}

// Status event handling.
void mainStatusCallback(Connection *from) {
    ESP_LOGI(TAG, "Connection status for %s changed to %s", from->peer, from->statusToName());
    
    if (hasCompanion && from->player == companion) {
        // Our companion lost connection.
        companion->connection->send("info", "leave");
        ESP_LOGI(TAG, "Disconn %s", from->player->getNick());
        setCompanion(NULL, false);
        currentScreen = Screen::HOME;
    }
    
    // Clear the asking out part.
    from->player->askedUsOut = false;
}
