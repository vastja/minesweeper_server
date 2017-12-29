#ifndef _IDLE_CHECKER_HPP_
#define _IDLE_CHECKER_HPP_

#include "Timer.hpp"
#include "Server.hpp"
#include "Client.hpp"

class IdleChecker : public BaseTimer {

    private:

        Server * server;
        Client ** clients;
        int clientsCount;
        Timer * timer;
        int maxIdleTime;

    public:

        static const int TIMEOUT = 2;
        static const int WAIT_TIME = 15;
        static const int MAX_IDLE_TIME = 60;
        static const int MAX_BAD_FORMAT_MESSAGES_IN_ROW = 5;

        IdleChecker(Server * server, Client ** clients, int clientsCount, int maxIdleTime = MAX_IDLE_TIME);
        ~IdleChecker();
        
        virtual void onAction();
        void start(int time = WAIT_TIME);
        void cancel();
        
};


#endif