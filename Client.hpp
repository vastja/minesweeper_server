#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

#include <time.h>

#include "BaseTimer.hpp"
#include "Timer.hpp"

class Client {

    private:
        int socket;
        const int id;
        time_t lastComunication;
        bool inGame;
        bool waitingForGame;
        bool marked;
        int badFormatMessages;
        
    public:

        Client(int id, int socket) : id(id) {
            this->socket = socket;
            lastComunication = time(0);
            waitingForGame = false;
            inGame = false;
            marked = false;
            badFormatMessages = 0;
        }

        inline void setSocket(int socket) {this->socket = socket;};
        int getClientsSocket() {return this->socket;};

        void setWaitingForGame(bool waiting) {this->waitingForGame = waiting;};
        bool isWaitingForGame() {return this->waitingForGame;};

        void setInGame(bool inGame) {this->inGame = inGame;};
        bool isInGame() {return this->inGame;};

        void setMarked(bool marked) {this->marked = marked;};
        bool isMarked() {return this->marked;};

        time_t getLastComunication() {return this->lastComunication;};
        void setLastCommunication() {time(&lastComunication);};

        const int getId() {return id;};

        void incBadFormatMessagesCount() {badFormatMessages++;};
        void resetBadFormatMessagesCount() {badFormatMessages = 0;};
        int getBadFormatMessagesCount() {return badFormatMessages;};

};

#endif