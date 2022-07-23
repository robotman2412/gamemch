
#pragma once

#define ENABLE_DEBUG
#define GAME_NVS_NAME "robot_spwn"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// For pin mappings.
#include "hardware.h"
// For graphics.
#include "pax_gfx.h"
#include "pax_shaders.h"
// For PNG images.
#include "pax_codecs.h"
// The screen driver.
#include "ili9341.h"
// For all system settings and alike.
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <esp_system.h>
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
// For WiFi connectivity.
#include "wifi_connect.h"
#include "wifi_connection.h"
// For exiting to the launcher.
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"
// Miscellaneous.
#include "time.h"
#include "sys/time.h"

// Game version, used to warn players running different versions.
#define GAME_VERSION 4

// Updates the screen with the last drawing.
void disp_flush();

// Exits the app, returning to the launcher.
void exit_to_launcher();

#ifdef __cplusplus
}
#endif //__cplusplus

typedef enum {
    // Intro screen: info about the game.
    INTRO,
    // Home screen: not doing much.
    HOME,
    // Companion select screen.
    COMP_SELECT,
    // We have asked the companion, await confirmation.
    COMP_AWAIT,
    // Pick a mutation.
    MUTATE_PICK,
    // Await the peer to pick a mutation.
    MUTATE_AWAIT,
} Screen;

extern Screen currentScreen;

#include <vector>

// For local connectivity.
#include "connection.h"
#include "espnowwrapper.h"
// Game stuff.
#include "player.h"
#include "graphics.h"

// The graphics buffer.
extern pax_buf_t buf;
// The queue which receives button events.
extern xQueueHandle buttonQueue;
// Info about you as a player.
extern Player *localPlayer;
// Info about the companion player.
extern Player *companion;
// Whether there is a (candidate) companion player.
extern bool hasCompanion;
// Whether the companion player has accepted.
extern bool companionAgrees;
// Whether the companion has finished mutating.
extern bool companionMutated;
// The list of players eligable to be a companion.
extern std::vector<int> companionList;
// The selected player in the companion list.
extern int companionListIndex;
// The mutated blobs the player picks from.
extern std::vector<Blob> candidates;
// The currently selected mutation candidate.
extern int candidateIndex;
// The title for the intro screen.
extern const char *introTitle;
// The text for the intro screen.
extern const char *introText;
// The scroll for the intro screen.
extern float introScroll;

extern Connection *broadcaster;

// Broadcast info obout ourselves.
void broadcastInfo();
// Ask a connection as companion.
void askCompanion(Connection *to);
// Sets the companion.
void setCompanion(Connection *to, bool agrees);
// Refreshes the list of candidate companions.
void refreshCompanionList();

// Create the candidate mutations.
void startMutation();
// Pick the mutation and await our peer.
void pickMutation();
// Finish the mutation thing.
void finishMutation();

// Data event handling.
void mainDataCallback(Connection *from, const char *type, const char *data);
// Status event handling.
void mainStatusCallback(Connection *from);
