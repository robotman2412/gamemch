
#pragma once

class Player {
    private:
        int lastUploadedScore;
        int score;
        char *nickname;
        
    public:
        // Make a new player.
        // Makes a copy of the nickname.
        Player(const char *nickname);
        
        // Handle a message for this player.
        void dataCallback(Connection *from, const char *type, const char *data);
        // Handle a connection status update for this player.
        void statusCallback(Connection *from);
        
        // Update a player's nickname.
        // Makes a copy of the nickname.
        void setNick(const char *newNickname);
        // Get a player's nickname.
        const char *getNick();
        
        // Update a player's score.
        int setScore(int newScore);
        // Get a player's score.
        int getScore();
};

// Loads a yourself as a player from NVS.
Player *loadFromNvs();
// Stores yourself as a player to NVS.
void storeToNvs(Player *player);
