
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
bool companionMutated;
Screen currentScreen;
std::vector<int> companionList;
int companionListIndex;
std::vector<Blob> candidates;
int candidateIndex;
float introScroll;

const char *introTitle = "Welcome to Spwn!";
const char *introText =
    "Spwn is a game about genes.\n\n"
    
    "You get a blob, which has moods\n"
    "and health. You cross horizontal-\n"
    "gene-transfer style with other\n"
    "players, but beware! The more you\n"
    "inbreed, the more sick you become!\n\n"
    
    "Spwn will show you nearby players,\n"
    "which you can ask to play with you.\n"
    "Press 🅴 when someone's nearby,\n"
    "walk up to them and ask them to\n"
    "do the same. Then, press 🆂 again\n"
    "to cross with their blob.\n\n"
    
    "Press 🆂 to continue to the game,\n"
    "or press 🅷 at any time to exit.\n";

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
    // Notify companion.
    if (hasCompanion) {
        companion->connection->send("info", "leave");
    }
    
    // Exit to launcher.
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
    // localPlayer->blob->pos.s0 = 0;
    // localPlayer->blob->pos.s1 = 0;
    // localPlayer->blob->pos.scale = 0;
    
    while (1) {
        espnow_run_callbacks();
        if (currentScreen == Screen::COMP_SELECT) {
            refreshCompanionList();
        }
        graphics_task();
        
        now = esp_timer_get_time() / 1000;
        
        if (now >= nextInfoBroadcast && currentScreen != Screen::INTRO) {
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
                introScroll = 0;
                
            } else if (message.input == RP2040_INPUT_JOYSTICK_DOWN && message.state) {
                if (currentScreen == Screen::HOME) {
                    // Debug: Next set.
                    if (debugAttrIndex < attributeSets.size() - 1) debugAttrIndex ++;
                }
                companionListIndex ++;
                introScroll = 1;
                
            } else if (message.input == RP2040_INPUT_JOYSTICK_RIGHT && message.state) {
                if (hasCompanion && candidates.size()) {
                    Blob *cur = &candidates[candidateIndex];
                    cur->pos.animateTo(cur->pos.x, cur->pos.y, 20, 500);
                    
                    candidateIndex ++;
                    if (candidateIndex >= candidates.size()) candidateIndex = 0;
                    
                    Blob *next = &candidates[candidateIndex];
                    next->pos.animateTo(next->pos.x, next->pos.y, 35, 500);
                }
                
            } else if (message.input == RP2040_INPUT_JOYSTICK_LEFT && message.state) {
                if (hasCompanion && candidates.size()) {
                    Blob *cur = &candidates[candidateIndex];
                    cur->pos.animateTo(cur->pos.x, cur->pos.y, 20, 500);
                    
                    candidateIndex --;
                    if (candidateIndex < 0) candidateIndex = candidates.size() - 1;
                    
                    Blob *next = &candidates[candidateIndex];
                    next->pos.animateTo(next->pos.x, next->pos.y, 35, 500);
                }
                
            } else if (message.input == RP2040_INPUT_BUTTON_ACCEPT && message.state) {
                #ifdef ENABLE_DEBUG
                if (currentScreen == Screen::HOME) {
                    // Debug: Toggle set.
                    localPlayer->blob->toggleSet(attributeSets[debugAttrIndex]);
                    localPlayer->blob->applyAttributes();
                    if (companion) {
                        localPlayer->blob->send(companion->connection);
                    }
                } else
                #endif
                if (currentScreen == Screen::COMP_SELECT) {
                    // Ask thel compañor.
                    currentScreen = Screen::COMP_AWAIT;
                    askCompanion(connections[companionList[companionListIndex]]);
                } else if (currentScreen == Screen::MUTATE_PICK) {
                    // Finish the mutation thing.
                    pickMutation();
                }
                
            } else if (message.input == RP2040_INPUT_BUTTON_START && message.state) {
                if (currentScreen == Screen::INTRO) {
                    currentScreen = Screen::HOME;
                    localPlayer->blob->pos.animateTo(buf.width/2, buf.height/2, 50, 1000);
                } else if (currentScreen == Screen::HOME && hasCompanion && companionAgrees) {
                    // Start the mutation procedure.
                    companion->connection->send("info", "mutate");
                    startMutation();
                }
                
            } else if (message.input == RP2040_INPUT_BUTTON_SELECT && message.state && !hasCompanion) {
                if (currentScreen == Screen::HOME && nearby == 1) {
                    // Just ask this one directly.
                    currentScreen = Screen::COMP_AWAIT;
                    askCompanion(firstNearby);
                } else if (currentScreen == Screen::HOME && nearby) {
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
                #ifdef ENABLE_DEBUG
                // Debug: Change blob with maximum random.
                localPlayer->blob->redoAttributes();
                if (companion) {
                    localPlayer->blob->send(companion->connection);
                }
                #endif
                
            } else if (message.input == RP2040_INPUT_BUTTON_MENU && message.state) {
                #ifdef ENABLE_DEBUG
                if (!hasCompanion) {
                    // Debug: Randomise again the attributes.
                    ESP_LOGI(TAG, "Randomising");
                    localPlayer->blob->attributes.clear();
                    localPlayer->blob->initialRandomise();
                }
                #endif
            }
        }
    }
}



// Create the candidate mutations.
void startMutation() {
    candidates.clear();
    companionMutated = false;
    
    // Animate away old blobs.
    localPlayer->blob->pos.animateTo(buf.width/3, buf.height/2, 0, 1000);
    companion->blob->pos.animateTo(buf.width*2/3, buf.height/2, 0, 1000);
    
    // Make new candidates.
    candidates.push_back(*localPlayer->blob);
    candidates[0].mutate(companion->blob);
    ESP_LOGE(TAG, "Candidate 0 picked");
    // Up to 5 tries for unique second candidate.
    candidates.push_back(*localPlayer->blob);
    for (int i = 0; i < 5 && (!i || candidates[1].setsEquals(candidates[0])); i++) {
        candidates[1] = *localPlayer->blob;
        candidates[1].mutate(companion->blob);
    }
    ESP_LOGE(TAG, "Candidate 1 picked");
    // Up to 10 tries for unique third candidate.
    candidates.push_back(*localPlayer->blob);
    for (int i = 0; i < 10 && (!i || candidates[2].setsEquals(candidates[0]) || candidates[2].setsEquals(candidates[1])); i++) {
        candidates[2] = *localPlayer->blob;
        candidates[2].mutate(companion->blob);
    }
    ESP_LOGE(TAG, "Candidate 2 picked");
    
    // Set candidate positions.
    candidates[0].pos.animateTo(320*0.25, 240*0.5, 20, 1000);
    candidates[1].pos.animateTo(320*0.50, 240*0.5, 35, 1000);
    candidates[2].pos.animateTo(320*0.75, 240*0.5, 20, 1000);
    
    candidateIndex = 1;
    currentScreen = Screen::MUTATE_PICK;
}

// Pick the mutation and await our peer.
void pickMutation() {
    // Await the pickening from other player.
    *localPlayer->blob = candidates[candidateIndex];
    currentScreen = Screen::MUTATE_AWAIT;
    
    // Animate back to normal.
    for (int i = 0; i < candidates.size(); i++) {
        if (i != candidateIndex) {
            candidates[i].pos.animateTo(candidates[i].pos.x, candidates[i].pos.y, 0, 1000);
        }
    }
    
    if (companionMutated) {
        // Don't await peer.
        finishMutation();
        companion->connection->send("info", "mutate_done");
    } else if (!hasCompanion) {
        // Finish early.
        finishMutation();
    } else {
        // Await peer.
        companion->connection->send("info", "mutate_done");
        localPlayer->blob->pos.animateTo(320*0.50, 240*0.5, 35, 1000);
    }
}

// Finish the mutation.
void finishMutation() {
    // Pick the mutat.
    candidateIndex = 0;
    candidates.clear();
    currentScreen = Screen::HOME;
    
    // Animate back to normal.
    localPlayer->blob->pos.animateTo(buf.width/3, buf.height/2, 35, 1000);
    companion->blob->pos.animateTo(buf.width*2/3, buf.height/2, 35, 1000);
    
    // Transmit data.
    localPlayer->blob->send(companion->connection);
    broadcastInfo();
}



// Broadcast info about ourselves.
void broadcastInfo() {
    // Advertise our presence by broadcasting our nickname and score.
    espnow_broadcast("nick",              localPlayer->getNick());
    espnow_broadcast_num("blob_body_col", localPlayer->blob->bodyColor);
    #ifdef ENABLE_DEBUG
    espnow_broadcast_num("version",       -GAME_VERSION);
    #else
    espnow_broadcast_num("version",       GAME_VERSION);
    #endif
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
                if (currentScreen == Screen::MUTATE_AWAIT) {
                    // Finish early.
                    finishMutation();
                }
            }
            from->player->askedUsOut = false;
        } else if (!strcmp(data, "mutate") && hasCompanion) {
            if (currentScreen == Screen::HOME) {
                // Temporary start mutating thing.
                startMutation();
            }
            
        } else if (!strcmp(data, "mutate_done") && hasCompanion) {
            if (currentScreen == Screen::MUTATE_PICK) {
                // Companion is done already, remember that.
                companionMutated = true;
            } else if (currentScreen == Screen::MUTATE_AWAIT) {
                // Companion is done, stop waiting.
                finishMutation();
            }
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
