#include "Client.hpp"

Client::Client(int id, int socket, Server * server) : id(id) {
    this->socket = socket;
    this->comunicationTimer = new Timer();
    this->server = server;
    lastComunication = time(0);
    warned = false;
    waitingForGame = false;
}

Client::~Client() {
    delete comunicationTimer;
}

void Client::onAction() {

    if (warned) {
        comunicationTimer->cancel();
        server->pushClientToIdleQueue(this);
    }
    else if (time(0) - lastComunication > 5 && !isWaitingForGame()) {
        server->executeAlive(this->id);
        warned = true;
        startTimer(EXTRA_TIME);
    }
    else {
        startTimer(NORMAL_TIME);
    }
   
}

void Client::startTimer(int time) {
    std::thread timer_thread (&Timer::start, comunicationTimer, time , this);
    timer_thread.detach();
}

void Client::cancelTimer() {
    comunicationTimer->cancel();
}