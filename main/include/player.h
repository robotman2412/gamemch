
#pragma once

class Player;
class Blob;

typedef enum {
    // Not currently interacting.
    IDLE,
    // Looking to interact (with or without companion).
    AWAITING,
    // Currently interacting with companion.
    INTERACTING,
} InteractStatus;

extern const char *interactStatusNames[3];
extern const char *fieldNames[22];
#define NUM_FIELDS 22

#include "connection.h"
#include "graphics.h"

class Player {
    private:
        int lastUploadedScore;
        int score;
        char *nickname;
        InteractStatus status;
        
    public:
        // Whether to broadcast status updates.
        bool doBroadcast;
        // The blob that belongs to this player.
        Blob *blob;
        // The (optional) connection for this player.
        Connection *connection;
        // Whether this player has asked us to be their companion.
        bool askedUsOut;
        
        // Make an empty player.
        Player();
        
        // Make a new player.
        // Makes a copy of the nickname.
        Player(const char *newNickname);
        
        ~Player();
        
        // Handle a message for this player.
        void dataCallback(Connection *from, const char *type, const char *data);
        // Handle a connection status update for this player.
        void statusCallback(Connection *from);
        
        // Update a player's nickname.
        // Makes a copy of the nickname.
        void setNick(const char *newNickname);
        // Get a player's nickname.
        const char *getNick();
        
        // Add to a player's score, negative is allowed but capped at 0.
        int addScore(int delta);
        // Set a player's score.
        int setScore(int newScore);
        // Get a player's score.
        int getScore();
        
        // Set interacting status.
        InteractStatus setStatus(InteractStatus newStatus);
        // Get interacting status.
        InteractStatus getStatus();
};

// Loads a yourself as a player from NVS.
Player *loadFromNvs();
// Stores yourself as a player to NVS.
void storeToNvs(Player *player);
// Handle a message for a player.
void playerDataCallback(Connection *from, const char *type, const char *data);
// Handle a connection status update for a player.
void playerStatusCallback(Connection *from);
