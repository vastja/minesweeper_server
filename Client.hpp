#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

#include "BaseTimer.hpp"
#include "Timer.hpp"
#include "Server.hpp"

class Client : public BaseTimer{

    private:
        int socket;
        const int id;
        Timer * comunicationTimer;
        Server * server;
        time_t lastComunication;
        std::thread timer_thread;
        bool warned;
        bool waitingForGame;
        
    public:

        static const int EXTRA_TIME = 5;
        static const int NORMAL_TIME = 2;

        Client(int id, int socket, Server * server);
        ~Client();
        
        // // TODO - should  be in private, should not?  
        void startTimer(int time);
        virtual void onAction();

        inline void setSocket(int socket) {this->socket = socket;};
        int getClientsSocket() {return this->socket;};

        void setWaitingForGame(bool waiting) {this->waitingForGame = waiting;};
        bool isWaitingForGame() {return this->waitingForGame;};

        int getLastComunication() {return lastComunication;};
        void setLastCommunication(int lastCommunication) {this->lastComunication = lastComunication; warned = false;};

        const int getId() {return id;};
        void cancelTimer();

};

#endif