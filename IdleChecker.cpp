#include "IdleChecker.hpp"
#include "Client.hpp"

IdleChecker::IdleChecker(Server * server, Client ** clients, int clientsCount, int maxIdleTime) : timer(new Timer()){
    this->server = server;
    this->clientsCount = clientsCount;
    this->clients = clients;
    this->maxIdleTime = maxIdleTime;
}

IdleChecker::~IdleChecker() {
    delete timer;
    timer = NULL;
}

void IdleChecker::start(int time) {

    std::thread timerThread (&Timer::start, timer, time, this);
    timerThread.detach();

};

void IdleChecker::cancel() {
    timer->cancel();
};

void IdleChecker::onAction() {

    for (int i = 0; i < clientsCount; i++) {

        if (clients[i] != NULL) {
            if (!(clients[i]->isInGame() || clients[i]->isWaitingForGame()) && !clients[i]->isMarked() && ((time(0) - clients[i]->getLastComunication()) > MAX_IDLE_TIME)) {
                clients[i]->setMarked(true);
                server->pushClientToIdleQueue(clients[i]->getId());
            }
            if (clients[i]->getBadFormatMessagesCount() >= MAX_BAD_FORMAT_MESSAGES_IN_ROW) {
                clients[i]->setMarked(true);
                server->pushClientToIdleQueue(clients[i]->getId());
            }
        }

    }

    start();

}