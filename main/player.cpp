
#include "player.h"
#include "string.h"
#include "stdlib.h"
#include "nvs.h"
#include "espnowwrapper.h"



const char *fieldNames[22] = {
    "Alexander Field",
    "Babbage Field",
    "Boole Field",
    "Clarke Field",
    "Engelbart Field",
    "Flowers Field",
    "Goldwasser Field",
    "Hamilton Field",
    "Hopper Field",
    "Lamarr Field",
    "Liskov Field",
    "Lovelace Field",
    "Manning Field",
    "Olsen Field",
    "Rhodes Field",
    "Snowden Field",
    "Titulaer Field",
    "Torvalds Field",
    "Turing Field",
    "Wilson Field",
    "Wozniak Field",
    "Zuse Field",
};

const char *interactStatusNames[3] = {
    "idle",
    "awaiting",
    "interacting"
};



// Loads a yourself as a player from NVS.
Player *loadFromNvs() {
    // Open NVS to read nickname.
    nvs_handle_t nick_handle;
    nvs_open("owner", NVS_READONLY, &nick_handle);
    // Allocate memory for nickname.
    size_t required = 0;
    nvs_get_str(nick_handle, "nickname", NULL, &required);
    if (required < 32) required = 32;
    char *nick = new char[required];
    *nick = 0;
    // Read nickname.
    nvs_get_str(nick_handle, "nickname", nick, &required);
    nvs_close(nick_handle);
    if (!*nick) strcpy(nick, "Anonymous");
    
    // Create player.
    Player *player = new Player(nick);
    // Because this is you as a player, broadcast all updates.
    player->doBroadcast = true;
    
    // Set the BLOB.
    player->blob = new Blob();
    player->blob->initialRandomise();
    
    // Set the version.
    player->version = GAME_VERSION;
    #ifdef ENABLE_DEBUG
    player->versionDebug = true;
    #else
    player->versionDebug = false;
    #endif
    player->updateVerStr();
    
    // Clean up.
    delete nick;
    return player;
}

// Stores yourself as a player to NVS.
void storeToNvs(Player *player) {
    
}



// Make an empty player.
Player::Player() {
    score        = 0;
    nickname     = strdup("Player");
    status       = InteractStatus::IDLE;
    doBroadcast  = false;
    blob         = new Blob();
    connection   = NULL;
    askedUsOut   = false;
    // Default version is v1 debug, because v2 is the first to track versions.
    version      = 1;
    versionDebug = true;
    strcpy(verStr, "v1 prerelease");
}

// Make a new player.
// Makes a copy of the nickname.
Player::Player(const char *newNickname) {
    score       = 0;
    nickname    = strdup(newNickname);
    status      = InteractStatus::IDLE;
    doBroadcast = false;
    connection  = NULL;
    askedUsOut  = false;
    // Default version is v1 debug, because v2 is the first to track versions.
    version      = 1;
    versionDebug = true;
    strcpy(verStr, "v1 prerelease");
}

Player::~Player() {
    free(nickname);
    delete blob;
    // It is intentional not to delete the connection.
}



// Handle a message for this player.
void Player::dataCallback(Connection *from, const char *type, const char *data) {
    if (!strcmp(type, "nick")) {
        // Literally any nickname is valid.
        setNick(data);
    } else if (!strcmp(type, "version")) {
        // Receive version.
        int num      = atoi(data);
        versionDebug = num < 0;
        version      = num < 0 ? -num : num;
        updateVerStr();
    } else if (!strncmp(type, "blob_", 5)) {
        // Data for the blob.
        blob->receive(from, type, data);
    } else if (!strcmp(type, "info") && !strcmp(data, "blob")) {
        if (from->player == companion) {
            // Send full blob.
            localPlayer->blob->send(from);
        } else {
            // Only send blob color.
            from->sendNum("blob_body_col", localPlayer->blob->bodyColor);
        }
    }
}

// Handle a connection status update for this player.
void Player::statusCallback(Connection *from) {
    // TODO
}



// Update a player's nickname.
// Makes a copy of the nickname.
void Player::setNick(const char *newNickname) {
    char *toFree = nickname;
    nickname = strdup(newNickname);
    free(toFree);
    if (doBroadcast) {
        espnow_broadcast("nick", nickname);
    }
}

// Get a player's nickname.
const char *Player::getNick() {
    return nickname;
}



// Add to a player's score, negative is allowed but capped at 0.
int Player::addScore(int delta) {
    score += delta;
    if (score < 0) score = 0;
    if (doBroadcast) {
        espnow_broadcast_num("score", score);
    }
    return score;
}

// Update a player's score.
int Player::setScore(int newScore) {
    score = newScore;
    if (score < 0) score = 0;
    if (doBroadcast) {
        espnow_broadcast_num("score", score);
    }
    return score;
}

// Get a player's score.
int Player::getScore() {
    return score;
}

// Update the string representation of version.
void Player::updateVerStr() {
    snprintf(verStr, 32, versionDebug ? "v%d prerelease" : "v%d", version);
}



// Set interacting status.
InteractStatus Player::setStatus(InteractStatus newStatus) {
    status = newStatus;
    if (doBroadcast) {
        espnow_broadcast("status", interactStatusNames[status]);
    }
    return status;
}

// Get interacting status.
InteractStatus Player::getStatus() {
    return status;
}



// Handle a message for a player.
void playerDataCallback(Connection *from, const char *type, const char *data) {
    from->player->dataCallback(from, type, data);
}

// Handle a connection status update for a player.
void playerStatusCallback(Connection *from) {
    from->player->statusCallback(from);
}
